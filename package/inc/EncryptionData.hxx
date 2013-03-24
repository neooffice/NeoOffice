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
#ifndef _ENCRYPTION_DATA_HXX_
#define _ENCRYPTION_DATA_HXX_

#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/weak.hxx>

#ifndef NO_OOO_3_4_1_AES_ENCRYPTION

// Digest types
#define ENCRYPTION_DATA_SHA1 ( (sal_Int32)1 )
#define ENCRYPTION_DATA_SHA256 ( (sal_Int32)2 )
#define ENCRYPTION_DATA_SHA1_1K ( (sal_Int32)3 )
#define ENCRYPTION_DATA_SHA256_1K ( (sal_Int32)4 )

// Algorithms
#define ENCRYPTION_DATA_AES_CBC_W3C_PADDING ( (sal_Int32)1 )
#define ENCRYPTION_DATA_BLOWFISH_CFB_8 ( (sal_Int32)2 )

#endif	// !NO_OOO_3_4_1_AES_ENCRYPTION

class EncryptionData : public cppu::OWeakObject
{
public:
	// On export aKey holds the derived key
	// On import aKey holds the hash of the user enterred key
	com::sun::star::uno::Sequence < sal_Int8 > aKey;
	com::sun::star::uno::Sequence < sal_uInt8 > aSalt, aInitVector, aDigest;
	sal_Int32 nIterationCount;
#ifdef NO_OOO_3_4_1_AES_ENCRYPTION
	EncryptionData(): nIterationCount ( 0 ){}
#else	// NO_OOO_3_4_1_AES_ENCRYPTION
	sal_Int32 nDigestType;
	sal_Int32 nAlgorithm;
	sal_Int32 nDerivedKeySize;
	sal_Int32 nStartKeyAlgorithm;
	EncryptionData(): nIterationCount( 0 ), nDigestType( 0 ), nAlgorithm( 0 ), nDerivedKeySize( 0 ), nStartKeyAlgorithm( 0 ) {}
#endif	// NO_OOO_3_4_1_AES_ENCRYPTION
};
#endif
