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
 * Modified January 2013 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_jvmfwk.hxx"

#include "osl/thread.h"
#include "otherjre.hxx"

#ifdef MACOSX

#include "sunversion.hxx"
#include "diagnostics.h"

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )

#endif	// MACOSX

using namespace rtl;
using namespace std;


namespace jfw_plugin
{

Reference<VendorBase> OtherInfo::createInstance()
{
    return new OtherInfo;
}


char const* const* OtherInfo::getJavaExePaths(int * size)
{
    static char const * ar[] = {
#if defined(WNT) || defined(OS2)
        "bin/java.exe",
        "jre/bin/java.exe"
#elif UNX
        "bin/java",
        "jre/bin/java"
#endif
    };
        *size = sizeof (ar) / sizeof (char*);
    return ar;
}

char const* const* OtherInfo::getRuntimePaths(int * size)
{
    static char const* ar[]= {
#if defined(WNT) || defined(OS2)
        "/bin/client/jvm.dll",
        "/bin/hotspot/jvm.dll",
        "/bin/classic/jvm.dll",
	"/bin/jrockit/jvm.dll"
#elif UNX
#ifdef MACOSX
        // Fix bug 1257 by explicitly loading the JVM instead of loading the
        // shared JavaVM library
        "/../Libraries/libserver.dylib"
#else
        "/lib/" JFW_PLUGIN_ARCH "/client/libjvm.so", // for Blackdown PPC
        "/lib/" JFW_PLUGIN_ARCH "/server/libjvm.so", // for Blackdown AMD64
        "/lib/" JFW_PLUGIN_ARCH "/classic/libjvm.so", // for Blackdown PPC
        "/lib/" JFW_PLUGIN_ARCH "/jrockit/libjvm.so", // for Java of BEA Systems
        "/bin/classic/libjvm.so", // fallback for older for IBM Java
        "/jre/bin/classic/libjvm.so" // fallback for older for IBM Java
#endif
#endif

    };
    *size = sizeof(ar) / sizeof (char*);
    return ar;
}

char const* const* OtherInfo::getLibraryPaths(int* size)
{

#ifdef UNX        
    static char const * ar[] = {
#ifdef MACOSX
        "/../Libraries",
        "/lib"
#else
        "/bin",
        "/jre/bin",
        "/bin/classic",
        "/jre/bin/classic",
        "/lib/" JFW_PLUGIN_ARCH "/client",
        "/lib/" JFW_PLUGIN_ARCH "/server",
        "/lib/" JFW_PLUGIN_ARCH "/classic",
        "/lib/" JFW_PLUGIN_ARCH "/jrockit",
        "/lib/" JFW_PLUGIN_ARCH "/native_threads",
        "/lib/" JFW_PLUGIN_ARCH
#endif
    };

    *size = sizeof(ar) / sizeof (char*);
    return ar;
#else
    size = 0;
    return NULL;
#endif
}

#ifdef MACOSX
int OtherInfo::compareVersions(const rtl::OUString& sSecond) const
#else	// MACOSX
int OtherInfo::compareVersions(const rtl::OUString& /*sSecond*/) const
#endif	// MACOSX
{
#ifdef MACOSX
    OUString sFirst = getVersion();
      
    SunVersion version1(sFirst);
    JFW_ENSURE(version1, OUSTR("[Java framework] sunjavaplugin"SAL_DLLEXTENSION
                               " does not know the version: ")
               + sFirst + OUSTR(" as valid for a SUN JRE."));
#ifdef USE_JAVA
    // Only run Java 1.6.x or later
    if ( version1 < SunVersion( ::rtl::OUString::createFromAscii( "1.6.0" ) ) )
        return -1;
#endif	// USE_JAVA

    SunVersion version2(sSecond);
    if ( ! version2)
        throw MalformedVersionException(); 
 
    if(version1 == version2)
        return 0;
    if(version1 > version2)
        return 1;
    else
        return -1;
#else	// MACOSX
    //Need to provide an own algorithm for comparing version. 
    //Because this function returns always 0, which means the version of
    //this JRE and the provided version "sSecond" are equal, one cannot put
    //any excludeVersion entries in the javavendors.xml file.
    return 0;
#endif	// MACOSX
}

}
