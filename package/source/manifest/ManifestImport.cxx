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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_package.hxx"
#include <ManifestImport.hxx>
#include <ManifestDefines.hxx>
#ifndef _BASE64_CODEC_HXX_
#include <Base64Codec.hxx>
#endif
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

#ifndef NO_OOO_3_4_1_AES_ENCRYPTION
#include "EncryptionData.hxx"
#endif	// !NO_OOO_3_4_1_AES_ENCRYPTION

using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star;
using namespace rtl;
using namespace std;

ManifestImport::ManifestImport( vector < Sequence < PropertyValue > > & rNewManVector )
: nNumProperty (0)
#ifndef NO_OOO_3_4_1_AES_ENCRYPTION
, nDerivedKeySize( 0 )
#endif	// !NO_OOO_3_4_1_AES_ENCRYPTION
, bIgnoreEncryptData 	( sal_False )
, rManVector ( rNewManVector )

, sFileEntryElement   	( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_FILE_ENTRY ) )
, sManifestElement    	( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_MANIFEST ) )
, sEncryptionDataElement( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_ENCRYPTION_DATA ) )
, sAlgorithmElement	( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_ALGORITHM ) )
, sKeyDerivationElement( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_KEY_DERIVATION ) )
#ifndef NO_OOO_3_4_1_AES_ENCRYPTION
, sStartKeyAlgorithmElement( RTL_CONSTASCII_USTRINGPARAM ( ELEMENT_START_KEY_GENERATION ) )
#endif	// !NO_OOO_3_4_1_AES_ENCRYPTION

, sCdataAttribute     			( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CDATA ) )
, sMediaTypeAttribute 			( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_MEDIA_TYPE ) )
, sVersionAttribute 			( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_VERSION ) )
, sFullPathAttribute  			( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_FULL_PATH ) )
, sSizeAttribute 				( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_SIZE ) )
, sSaltAttribute 				( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_SALT ) )
, sInitialisationVectorAttribute ( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_INITIALISATION_VECTOR ) )
, sIterationCountAttribute 		( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_ITERATION_COUNT ) )
, sAlgorithmNameAttribute 		( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_ALGORITHM_NAME ) )
, sKeyDerivationNameAttribute 	( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_KEY_DERIVATION_NAME ) )
, sChecksumAttribute 			( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CHECKSUM ) )
, sChecksumTypeAttribute 		( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_CHECKSUM_TYPE ) )

, sFullPathProperty  			( RTL_CONSTASCII_USTRINGPARAM ( "FullPath" ) )
, sMediaTypeProperty 			( RTL_CONSTASCII_USTRINGPARAM ( "MediaType" ) )
, sVersionProperty  			( RTL_CONSTASCII_USTRINGPARAM ( "Version" ) )
, sIterationCountProperty 		( RTL_CONSTASCII_USTRINGPARAM ( "IterationCount" ) )
, sSaltProperty 				( RTL_CONSTASCII_USTRINGPARAM ( "Salt" ) )
, sInitialisationVectorProperty	( RTL_CONSTASCII_USTRINGPARAM ( "InitialisationVector" ) )
, sSizeProperty 				( RTL_CONSTASCII_USTRINGPARAM ( "Size" ) )
, sDigestProperty 				( RTL_CONSTASCII_USTRINGPARAM ( "Digest" ) )
#ifndef NO_OOO_3_4_1_AES_ENCRYPTION
, sDigestTypeProperty 			( RTL_CONSTASCII_USTRINGPARAM ( "DigestType" ) )
, sAlgorithmProperty			( RTL_CONSTASCII_USTRINGPARAM ( "Algorithm" ) )
, sDerivedKeySizeProperty		( RTL_CONSTASCII_USTRINGPARAM ( "DerivedKeySize" ) )
, sStartKeyAlgorithmProperty	( RTL_CONSTASCII_USTRINGPARAM ( "StartKeyAlgorithm" ) )
#endif	// !NO_OOO_3_4_1_AES_ENCRYPTION

, sWhiteSpace 					( RTL_CONSTASCII_USTRINGPARAM ( " " ) )
, sBlowfish						( RTL_CONSTASCII_USTRINGPARAM ( "Blowfish CFB" ) )
, sPBKDF2 						( RTL_CONSTASCII_USTRINGPARAM ( "PBKDF2" ) )
#ifdef NO_OOO_3_4_1_AES_ENCRYPTION
, sChecksumType					( RTL_CONSTASCII_USTRINGPARAM ( CHECKSUM_TYPE ) )
#endif	// NO_OOO_3_4_1_AES_ENCRYPTION
{
}
ManifestImport::~ManifestImport (void )
{
}
void SAL_CALL ManifestImport::startDocument(  ) 	
		throw(xml::sax::SAXException, uno::RuntimeException)
{
}
void SAL_CALL ManifestImport::endDocument(  ) 	
		throw(xml::sax::SAXException, uno::RuntimeException)
{
}
void SAL_CALL ManifestImport::startElement( const OUString& aName, const uno::Reference< xml::sax::XAttributeList >& xAttribs ) 	
		throw(xml::sax::SAXException, uno::RuntimeException)
{
	if (aName == sFileEntryElement)
	{
		aStack.push( e_FileEntry );
		aSequence.realloc ( PKG_SIZE_ENCR_MNFST );

		// Put full-path property first for MBA
		aSequence[nNumProperty].Name = sFullPathProperty;
		aSequence[nNumProperty++].Value <<= xAttribs->getValueByName( sFullPathAttribute );
		aSequence[nNumProperty].Name = sMediaTypeProperty;
		aSequence[nNumProperty++].Value <<= xAttribs->getValueByName( sMediaTypeAttribute );

		OUString sVersion = xAttribs->getValueByName ( sVersionAttribute );
        if ( sVersion.getLength() )
        {
            aSequence[nNumProperty].Name = sVersionProperty;
            aSequence[nNumProperty++].Value <<= sVersion;
        }

		OUString sSize = xAttribs->getValueByName ( sSizeAttribute );
		if (sSize.getLength())
		{
			sal_Int32 nSize;
			nSize = sSize.toInt32();
			aSequence[nNumProperty].Name = sSizeProperty;
			aSequence[nNumProperty++].Value <<= nSize;
		}
	}
	else if (!aStack.empty())
	{
		if (aStack.top() == e_FileEntry && aName == sEncryptionDataElement)
		{
			// If this element exists, then this stream is encrypted and we need
			// to store the initialisation vector, salt and iteration count used
			aStack.push (e_EncryptionData );
#ifdef NO_OOO_3_4_1_AES_ENCRYPTION
			OUString aString = xAttribs->getValueByName ( sChecksumTypeAttribute );
			if (aString == sChecksumType && !bIgnoreEncryptData)
			{
				aString = xAttribs->getValueByName ( sChecksumAttribute );
				Sequence < sal_uInt8 > aDecodeBuffer;
				Base64Codec::decodeBase64 (aDecodeBuffer, aString);
				aSequence[nNumProperty].Name = sDigestProperty;
				aSequence[nNumProperty++].Value <<= aDecodeBuffer;
			}
#else	// NO_OOO_3_4_1_AES_ENCRYPTION
            nDerivedKeySize = 0;
            if ( !bIgnoreEncryptData )
            {
                sal_Int32 nDigestId = 0;
                OUString aChecksumType = xAttribs->getValueByName( sChecksumTypeAttribute );
                if ( aChecksumType.equalsAscii( SHA1_1K_NAME ) || aChecksumType.equalsAscii( SHA1_1K_URL ) )
                    nDigestId = ENCRYPTION_DATA_SHA1_1K;
                else if ( aChecksumType.equalsAscii( SHA256_1K_URL ) )
                    nDigestId = ENCRYPTION_DATA_SHA256_1K;
                else
                    bIgnoreEncryptData = true;

                if ( !bIgnoreEncryptData )
                {
                    OUString aChecksum = xAttribs->getValueByName( sChecksumAttribute );
                    Sequence< sal_uInt8 > aDecodeBuffer;
                    Base64Codec::decodeBase64( aDecodeBuffer, aChecksum );
                    aSequence[nNumProperty].Name = sDigestProperty;
                    aSequence[nNumProperty++].Value <<= aDecodeBuffer;
                    aSequence[nNumProperty].Name = sDigestTypeProperty;
                    aSequence[nNumProperty++].Value <<= nDigestId;
                }
            }
#endif	// NO_OOO_3_4_1_AES_ENCRYPTION
		}
		else if (aStack.top() == e_EncryptionData && aName == sAlgorithmElement)
		{
			aStack.push (e_Algorithm);
#ifdef NO_OOO_3_4_1_AES_ENCRYPTION
			OUString aString = xAttribs->getValueByName ( sAlgorithmNameAttribute );
			if (aString == sBlowfish && !bIgnoreEncryptData)
			{
				aString = xAttribs->getValueByName ( sInitialisationVectorAttribute );
				Sequence < sal_uInt8 > aDecodeBuffer;
				Base64Codec::decodeBase64 (aDecodeBuffer, aString);
				aSequence[nNumProperty].Name = sInitialisationVectorProperty;
				aSequence[nNumProperty++].Value <<= aDecodeBuffer;
			}
			else
				// If we don't recognise the algorithm, then the key derivation info
				// is useless to us
				bIgnoreEncryptData = sal_True;
#else	// NO_OOO_3_4_1_AES_ENCRYPTION
            if ( !bIgnoreEncryptData )
            {
                sal_Int32 nCypherId = 0;
                OUString aAlgoName = xAttribs->getValueByName( sAlgorithmNameAttribute );
                if ( aAlgoName.equalsAscii( BLOWFISH_NAME ) || aAlgoName.equalsAscii( BLOWFISH_URL ) )
                {
                	nCypherId = ENCRYPTION_DATA_BLOWFISH_CFB_8;
                }
                else if( aAlgoName.equalsAscii( AES256_URL ) )
                {
                	nCypherId = ENCRYPTION_DATA_AES_CBC_W3C_PADDING;
                    OSL_ENSURE( !nDerivedKeySize || nDerivedKeySize == 32, "Unexpected derived key length!" );
                    nDerivedKeySize = 32;
                }
                else if ( aAlgoName.equalsAscii( AES192_URL ) )
                {
                	nCypherId = ENCRYPTION_DATA_AES_CBC_W3C_PADDING;
                    OSL_ENSURE( !nDerivedKeySize || nDerivedKeySize == 24, "Unexpected derived key length!" );
                    nDerivedKeySize = 24;
                }
                else if ( aAlgoName.equalsAscii( AES128_URL ) )
                {
                	nCypherId = ENCRYPTION_DATA_AES_CBC_W3C_PADDING;
                    OSL_ENSURE( !nDerivedKeySize || nDerivedKeySize == 16, "Unexpected derived key length!" );
                    nDerivedKeySize = 16;
                }
                else
                {
                    bIgnoreEncryptData = true;
                }

                if ( !bIgnoreEncryptData )
                {
                    OUString aAlgoInitVector = xAttribs->getValueByName( sInitialisationVectorAttribute );
                    Sequence< sal_uInt8 > aDecodeBuffer;
                    Base64Codec::decodeBase64( aDecodeBuffer, aAlgoInitVector );
                    aSequence[nNumProperty].Name = sInitialisationVectorProperty;
                    aSequence[nNumProperty++].Value <<= aDecodeBuffer;
                    aSequence[nNumProperty].Name = sAlgorithmProperty;
                    aSequence[nNumProperty++].Value <<= nCypherId;
                }
            }
#endif	// NO_OOO_3_4_1_AES_ENCRYPTION
		}
		else if (aStack.top() == e_EncryptionData && aName == sKeyDerivationElement)
		{
			aStack.push (e_KeyDerivation);
#ifdef NO_OOO_3_4_1_AES_ENCRYPTION
			OUString aString = xAttribs->getValueByName ( sKeyDerivationNameAttribute );
			if ( aString == sPBKDF2 && !bIgnoreEncryptData )
			{
				aString = xAttribs->getValueByName ( sSaltAttribute );
				Sequence < sal_uInt8 > aDecodeBuffer;
				Base64Codec::decodeBase64 (aDecodeBuffer, aString);
				aSequence[nNumProperty].Name = sSaltProperty;
				aSequence[nNumProperty++].Value <<= aDecodeBuffer;

				aString = xAttribs->getValueByName ( sIterationCountAttribute );
				aSequence[nNumProperty].Name = sIterationCountProperty;
				aSequence[nNumProperty++].Value <<= aString.toInt32();
			}
			else
				// If we don't recognise the key derivation technique, then the
				// algorithm info is useless to us
				bIgnoreEncryptData = sal_True;
#else	// NO_OOO_3_4_1_AES_ENCRYPTION
            if ( !bIgnoreEncryptData )
            {
                OUString aKeyDerivString = xAttribs->getValueByName( sKeyDerivationNameAttribute );
                if ( aKeyDerivString.equalsAscii( PBKDF2_NAME ) || aKeyDerivString.equalsAscii( PBKDF2_URL ) )
                {
				    OUString aString = xAttribs->getValueByName( sSaltAttribute );
                    Sequence< sal_uInt8 > aDecodeBuffer;
                    Base64Codec::decodeBase64( aDecodeBuffer, aString );
                    aSequence[nNumProperty].Name = sSaltProperty;
                    aSequence[nNumProperty++].Value <<= aDecodeBuffer;

                    aString = xAttribs->getValueByName ( sIterationCountAttribute );
                    aSequence[nNumProperty].Name = sIterationCountProperty;
                    aSequence[nNumProperty++].Value <<= aString.toInt32();

				    OUString aKeySize = xAttribs->getValueByName( OUString( RTL_CONSTASCII_USTRINGPARAM ( ATTRIBUTE_KEY_SIZE ) ) );
                    if ( aKeySize.getLength() )
                    {
                        const sal_Int32 nKey = aKeySize.toInt32();
                        OSL_ENSURE( !nDerivedKeySize || nKey == nDerivedKeySize , "Provided derived key length differs from the expected one!" );
                        nDerivedKeySize = nKey;
                    }
                    else if ( !nDerivedKeySize )
                    {
                        nDerivedKeySize = 16;
                    }
                    else if ( nDerivedKeySize != 16 )
                    {
                        OSL_ENSURE( sal_False, "Default derived key length differs from the expected one!" );
                    }

                    aSequence[nNumProperty].Name = sDerivedKeySizeProperty;
                    aSequence[nNumProperty++].Value <<= nDerivedKeySize;
                }
                else
                {
                    bIgnoreEncryptData = true;
                }
            }
#endif	// NO_OOO_3_4_1_AES_ENCRYPTION
		}
#ifndef NO_OOO_3_4_1_AES_ENCRYPTION
		else if (aStack.top() == e_EncryptionData && aName == sKeyDerivationElement)
		{
			aStack.push (e_StartKeyAlgorithm);
            if ( !bIgnoreEncryptData )
            {
                OUString aStartKeyAlgString = xAttribs->getValueByName( sStartKeyAlgorithmElement );
                if ( aStartKeyAlgString.equalsAscii( SHA1_NAME ) || aStartKeyAlgString.equalsAscii( SHA1_URL ) )
                {
                    aSequence[nNumProperty].Name = sStartKeyAlgorithmProperty;
                    aSequence[nNumProperty++].Value <<= ENCRYPTION_DATA_SHA1;
                }
                else if ( aStartKeyAlgString.equalsAscii( SHA256_URL ) )
                {
                    aSequence[nNumProperty].Name = sStartKeyAlgorithmProperty;
                    aSequence[nNumProperty++].Value <<= ENCRYPTION_DATA_SHA256;
                }
                else
                {
                    bIgnoreEncryptData = true;
                }
            }
		}
#endif	// !NO_OOO_3_4_1_AES_ENCRYPTION
	}
}
void SAL_CALL ManifestImport::endElement( const OUString& /*aName*/ ) 	
	throw(xml::sax::SAXException, uno::RuntimeException)
{
	if ( !aStack.empty() )
	{ 
		if (aStack.top() == e_FileEntry)
		{
			aSequence.realloc ( nNumProperty );
			bIgnoreEncryptData = sal_False;
			rManVector.push_back ( aSequence );
			nNumProperty = 0;
		}
		aStack.pop();
	}
}
void SAL_CALL ManifestImport::characters( const OUString& /*aChars*/ ) 	
		throw(xml::sax::SAXException, uno::RuntimeException)
{
}
void SAL_CALL ManifestImport::ignorableWhitespace( const OUString& /*aWhitespaces*/ ) 	
		throw(xml::sax::SAXException, uno::RuntimeException)
{
}
void SAL_CALL ManifestImport::processingInstruction( const OUString& /*aTarget*/, const OUString& /*aData*/ ) 	
		throw(xml::sax::SAXException, uno::RuntimeException)
{
}
void SAL_CALL ManifestImport::setDocumentLocator( const uno::Reference< xml::sax::XLocator >& /*xLocator*/ ) 	
		throw(xml::sax::SAXException, uno::RuntimeException)
{
}

