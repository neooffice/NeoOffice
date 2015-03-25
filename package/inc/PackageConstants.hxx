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
#ifndef _PACKAGE_CONSTANTS_HXX_
#define _PACKAGE_CONSTANTS_HXX_

#include <sal/types.h>

const sal_Int32 n_ConstBufferSize = 32768;
const sal_Int32 n_ConstMaxMemoryStreamSize = 20480;
const sal_Int32 n_ConstDigestLength = 1024;

#define PACKAGE_FORMAT	1
#define ZIP_FORMAT		2
#define OFOPXML_FORMAT	3

// the constants related to the manifest.xml entries
#define PKG_MNFST_MEDIATYPE 0
#define PKG_MNFST_VERSION   1
#define PKG_MNFST_FULLPATH  2

#define PKG_MNFST_INIVECTOR 3
#define PKG_MNFST_SALT      4
#define PKG_MNFST_ITERATION 5
#define PKG_MNFST_UCOMPSIZE 6
#define PKG_MNFST_DIGEST    7
#ifndef NO_OOO_3_4_1_AES_ENCRYPTION
#define PKG_MNFST_ENCALG      8
#define PKG_MNFST_STARTALG    9
#define PKG_MNFST_DIGESTALG  10
#define PKG_MNFST_DERKEYSIZE 11
#endif	// !NO_OOO_3_4_1_AES_ENCRYPTION

#define PKG_SIZE_NOENCR_MNFST 3
#ifdef NO_OOO_3_4_1_AES_ENCRYPTION
#define PKG_SIZE_ENCR_MNFST   8
#else	// NO_OOO_3_4_1_AES_ENCRYPTION
#define PKG_SIZE_ENCR_MNFST  12
#endif	// NO_OOO_3_4_1_AES_ENCRYPTION


#endif

