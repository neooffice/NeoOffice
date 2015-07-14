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
 * Modified July 2015 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_jvmfwk.hxx"

#include "vendorlist.hxx"
#include "gnujre.hxx"
#include "sunjre.hxx"
#include "otherjre.hxx"
#include "osl/thread.h"
#include <stdio.h>

using namespace com::sun::star::uno;
using namespace rtl;

namespace jfw_plugin
{

/* Note: The vendor strings must be UTF-8. For example, if
   the string contains an a umlaut then it must be expressed
   by "\xXX\xXX"
 */
BEGIN_VENDOR_MAP()
    VENDOR_MAP_ENTRY("Sun Microsystems Inc.", SunInfo)
    VENDOR_MAP_ENTRY("IBM Corporation", OtherInfo)
    VENDOR_MAP_ENTRY("Blackdown Java-Linux Team", OtherInfo)
    VENDOR_MAP_ENTRY("Apple Inc.", OtherInfo)
    VENDOR_MAP_ENTRY("Apple Computer, Inc.", OtherInfo)
    VENDOR_MAP_ENTRY("BEA Systems, Inc.", OtherInfo)
    VENDOR_MAP_ENTRY("Free Software Foundation, Inc.", GnuInfo)
    VENDOR_MAP_ENTRY("The FreeBSD Foundation", OtherInfo)
#ifdef USE_JAVA
    VENDOR_MAP_ENTRY("Oracle Corporation", SunInfo)
#endif	// USE_JAVA
END_VENDOR_MAP()    


Sequence<OUString> getVendorNames()
{
    const size_t count = sizeof(gVendorMap) / sizeof (VendorSupportMapEntry) - 1;
    OUString arNames[count];
    for ( size_t pos = 0; pos < count; ++pos )
    {
        OString sVendor(gVendorMap[pos].sVendorName);
        arNames[pos] = OStringToOUString(sVendor, RTL_TEXTENCODING_UTF8);
    }
    return Sequence<OUString>(arNames, count);
}

bool isVendorSupported(const rtl::OUString& sVendor)
{
    Sequence<OUString> seqNames = getVendorNames();
    const OUString * arNames = seqNames.getConstArray();
    sal_Int32 count = seqNames.getLength();
    
    for (int i = 0; i < count; i++)
    {
        if (sVendor.equals(arNames[i]))
            return true;
    }
#if OSL_DEBUG_LEVEL >= 2
    OString sVendorName = OUStringToOString(sVendor, osl_getThreadTextEncoding());
    fprintf(stderr, "[Java frameworksunjavaplugin.so]sunjavaplugin does not support vendor: %s.\n",
            sVendorName.getStr());
#endif    
    return false;
}

}
