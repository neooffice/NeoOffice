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

#include "osl/thread.h"
#include "sunjre.hxx"
#include "sunversion.hxx"
#include "diagnostics.h"

using namespace rtl;
using namespace std;

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
namespace jfw_plugin
{

Reference<VendorBase> SunInfo::createInstance()
{
    return new SunInfo;
}

char const* const* SunInfo::getJavaExePaths(int * size)
{
    static char const * ar[] = {
#if defined(WNT) || defined(OS2)
        "java.exe",
        "bin/java.exe",
        "jre/bin/java.exe"
#elif UNX
        "java",
        "bin/java",
        "jre/bin/java"
#endif
    };
        *size = sizeof (ar) / sizeof (char*);
    return ar;
}

char const* const* SunInfo::getRuntimePaths(int * size)
{
    static char const* ar[]= {
#if defined(WNT)
        "/bin/client/jvm.dll",
        "/bin/hotspot/jvm.dll",
        "/bin/classic/jvm.dll"
#elif defined(OS2)
        "/bin/classic/jvm.dll",
        "/bin/client/jvm.dll",
        "/bin/hotspot/jvm.dll"
#elif UNX
#if defined USE_JAVA && defined MACOSX
        "/lib/server/libjvm.dylib",
        "/lib/jli/libjli.dylib"
#else	// USE_JAVA && MACOSX
        "/lib/" JFW_PLUGIN_ARCH "/client/libjvm.so",
        "/lib/" JFW_PLUGIN_ARCH "/server/libjvm.so",
        "/lib/" JFW_PLUGIN_ARCH "/classic/libjvm.so"
#endif	// USE_JAVA && MACOSX
#endif

    };
    *size = sizeof(ar) / sizeof (char*);
    return ar;
}

char const* const* SunInfo::getLibraryPaths(int* size)
{
#ifdef UNX        
    static char const * ar[] = {

        "/lib/" JFW_PLUGIN_ARCH "/client",
        "/lib/" JFW_PLUGIN_ARCH "/native_threads", 
        "/lib/" JFW_PLUGIN_ARCH

    };
    *size = sizeof(ar) / sizeof (char*);
    return ar;
#else
    size = 0;
    return NULL;
#endif
}

int SunInfo::compareVersions(const rtl::OUString& sSecond) const
{
    OUString sFirst = getVersion();
    
    SunVersion version1(sFirst);
    JFW_ENSURE(version1, OUSTR("[Java framework] sunjavaplugin"SAL_DLLEXTENSION
                               " does not know the version: ")
               + sFirst + OUSTR(" as valid for a SUN JRE."));
    SunVersion version2(sSecond);
    if ( ! version2)
        throw MalformedVersionException();

    if(version1 == version2)
        return 0;
    if(version1 > version2)
        return 1;
    else
        return -1;
}


}
