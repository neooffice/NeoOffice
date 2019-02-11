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
 * 
 *   Modified November 2016 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifdef _WIN32
# include <stdio.h>
# include <sys/stat.h>
# include <windows.h>
#endif

#ifdef ANDROID
# include <dlfcn.h>
#endif

#include <string.h>

#include <cassert>
#include <memory>
#include <utility>
#include <vector>

#include "config_options.h"
#include "osl/diagnose.h"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "osl/module.hxx"
#include "osl/mutex.hxx"
#include "osl/process.h"
#include "osl/thread.hxx"
#include "osl/file.hxx"
#include "rtl/instance.hxx"
#include "osl/getglobalmutex.hxx"
#include <setjmp.h>
#include <signal.h>
#include <stack>

#include "jni.h"
#include "rtl/byteseq.hxx"
#include "vendorplugin.hxx"
#include "util.hxx"
#include "sunversion.hxx"
#include "vendorlist.hxx"
#include "diagnostics.h"

#ifdef MACOSX
#include "util_cocoa.hxx"
#endif

#ifdef ANDROID
#include <osl/detail/android-bootstrap.h>
#else
#if !ENABLE_RUNTIME_OPTIMIZATIONS
#define FORCE_INTERPRETED 1
#elif defined HAVE_VALGRIND_HEADERS
#include <valgrind/valgrind.h>
#define FORCE_INTERPRETED RUNNING_ON_VALGRIND
#else
#define FORCE_INTERPRETED 0
#endif
#endif

#if defined LINUX && (defined X86 || defined X86_64)
#include <sys/resource.h>
#endif

using namespace osl;
using namespace std;
using namespace jfw_plugin;


namespace {

struct PluginMutex: public ::rtl::Static<osl::Mutex, PluginMutex> {};

#if defined(UNX) && !defined(ANDROID)
OString getPluginJarPath(
    const OUString & sVendor,
    const OUString& sLocation,
    const OUString& sVersion)
{
    OString ret;
    OUString sName1("javaplugin.jar");
    OUString sName2("plugin.jar");
    OUString sPath;
    if ( sVendor == "Sun Microsystems Inc." )
    {
        SunVersion ver142("1.4.2-ea");
        SunVersion ver150("1.5.0-ea");
        SunVersion ver(sVersion);
        OSL_ASSERT(ver142 && ver150 && ver);

        OUString sName;
        if (ver < ver142)
        {
            sName = sName1;
        }
        else if (ver < ver150)
        {//this will cause ea, beta etc. to have plugin.jar in path.
            //but this does not harm. 1.5.0-beta < 1.5.0
            sName = sName2;
        }
        if (!sName.isEmpty())
        {
            sName = sLocation + "/lib/" + sName;
            OSL_VERIFY(
                osl_getSystemPathFromFileURL(sName.pData, & sPath.pData)
                == osl_File_E_None);
        }
    }
    else
    {
        OUString sName(sLocation + "/lib/" + sName1);
        OUString sPath1;
        OUString sPath2;
        if (osl_getSystemPathFromFileURL(sName.pData, & sPath1.pData)
            == osl_File_E_None)
        {
            sName = sLocation + "/lib/" + sName2;
            if (osl_getSystemPathFromFileURL(sName.pData, & sPath2.pData)
                == osl_File_E_None)
            {
                char sep[] = {SAL_PATHSEPARATOR, 0};
                sPath = sPath1 + OUString::createFromAscii(sep) + sPath2;
            }
        }
        OSL_ASSERT(!sPath.isEmpty());
    }
    ret = OUStringToOString(sPath, osl_getThreadTextEncoding());

    return ret;
}
#endif // UNX


std::unique_ptr<JavaInfo> createJavaInfo(
    const rtl::Reference<VendorBase> & info)
{
    OUStringBuffer buf(1024);
    buf.append(info->getRuntimeLibrary());
    if (!info->getLibraryPath().isEmpty())
    {
        buf.append("\n");
        buf.append(info->getLibraryPath());
        buf.append("\n");
    }
    OUString sVendorData = buf.makeStringAndClear();
    return std::unique_ptr<JavaInfo>(
        new JavaInfo{
            info->getVendor(), info->getHome(), info->getVersion(),
            sal_uInt64(info->supportsAccessibility() ? 1 : 0),
            sal_uInt64(info->needsRestart() ? JFW_REQUIRE_NEEDRESTART : 0),
            rtl::ByteSequence(
                reinterpret_cast<sal_Int8*>(sVendorData.pData->buffer),
                sVendorData.getLength() * sizeof(sal_Unicode))});
}

OUString getRuntimeLib(const rtl::ByteSequence & data)
{
    const sal_Unicode* chars = reinterpret_cast<sal_Unicode const *>(data.getConstArray());
    sal_Int32 len = data.getLength();
    OUString sData(chars, len / 2);
    //the runtime lib is on the first line
    sal_Int32 index = 0;
    OUString aToken = sData.getToken( 0, '\n', index);

    return aToken;
}

jmp_buf jmp_jvm_abort;
sig_atomic_t g_bInGetJavaVM = 0;

extern "C" void JNICALL abort_handler()
{
    // If we are within JNI_CreateJavaVM then we jump back into getJavaVM
    if( g_bInGetJavaVM != 0 )
    {
        fprintf(stderr, "JavaVM: JNI_CreateJavaVM called os::abort(), caught by abort_handler in javavm.cxx\n");
        longjmp( jmp_jvm_abort, 0);
    }
}

/** helper function to check Java version requirements

    This function checks if the Java version of the given VendorBase
    meets the given Java version requirements.

    @param aVendorInfo
        [in]  the object to be inspected whether it meets the version requirements
    @param sMinVersion
        [in] represents the minimum version of a JRE. The string can be empty.
    @param sMaxVersion
        [in] represents the maximum version of a JRE. The string can be empty.
    @param arExcludeList
        [in] contains a list of &quot;bad&quot; versions. JREs which have one of these
        versions must not be returned by this function.

   @return
    javaPluginError::NONE the function ran successfully and the version requirements are met
    javaPluginError::FailedVersion at least one of the version requirements (minVersion,
    maxVersion, excludeVersions) was violated
    javaPluginError::WrongVersionFormat the version strings in
    <code>sMinVersion,sMaxVersion,arExcludeList</code> are not recognized as valid
    version strings.

     */
javaPluginError checkJavaVersionRequirements(
    rtl::Reference<VendorBase> const & aVendorInfo,
    OUString const& sMinVersion,
    OUString const& sMaxVersion,
    std::vector<OUString> const & arExcludeList)
{
    if (!aVendorInfo->isValidArch())
    {
        return javaPluginError::WrongArch;
    }
    if (!sMinVersion.isEmpty())
    {
        try
        {
            if (aVendorInfo->compareVersions(sMinVersion) < 0)
                return javaPluginError::FailedVersion;
        }
        catch (MalformedVersionException&)
        {
            //The minVersion was not recognized as valid for this vendor.
            JFW_ENSURE(
                false,
                "[Java framework]sunjavaplugin does not know version: "
                + sMinVersion + " for vendor: " + aVendorInfo->getVendor()
                + " .Check minimum Version." );
            return javaPluginError::WrongVersionFormat;
        }
    }

    if (!sMaxVersion.isEmpty())
    {
        try
        {
            if (aVendorInfo->compareVersions(sMaxVersion) > 0)
                return javaPluginError::FailedVersion;
        }
        catch (MalformedVersionException&)
        {
            //The maxVersion was not recognized as valid for this vendor.
            JFW_ENSURE(
                false,
                "[Java framework]sunjavaplugin does not know version: "
                + sMaxVersion + " for vendor: " + aVendorInfo->getVendor()
                + " .Check maximum Version." );
            return javaPluginError::WrongVersionFormat;
        }
    }

    for (auto const & sExVer: arExcludeList) {
        try
        {
            if (aVendorInfo->compareVersions(sExVer) == 0)
                return javaPluginError::FailedVersion;
        }
        catch (MalformedVersionException&)
        {
            //The excluded version was not recognized as valid for this vendor.
            JFW_ENSURE(
                false,
                "[Java framework]sunjavaplugin does not know version: "
                + sExVer + " for vendor: " + aVendorInfo->getVendor()
                + " .Check excluded versions." );
            return javaPluginError::WrongVersionFormat;
        }
    }

    return javaPluginError::NONE;
}

}

javaPluginError jfw_plugin_getAllJavaInfos(
    bool checkJavaHomeAndPath,
    OUString const& sVendor,
    OUString const& sMinVersion,
    OUString const& sMaxVersion,
    std::vector<OUString> const &arExcludeList,
    std::vector<std::unique_ptr<JavaInfo>>* parJavaInfo,
    std::vector<rtl::Reference<jfw_plugin::VendorBase>> & infos)
{
    assert(parJavaInfo);

    OSL_ASSERT(!sVendor.isEmpty());
    if (sVendor.isEmpty())
        return javaPluginError::InvalidArg;

    //Find all JREs
    vector<rtl::Reference<VendorBase> > vecInfos =
        addAllJREInfos(checkJavaHomeAndPath, infos);
    vector<rtl::Reference<VendorBase> > vecVerifiedInfos;

    typedef vector<rtl::Reference<VendorBase> >::iterator it;
    for (it i= vecInfos.begin(); i != vecInfos.end(); ++i)
    {
        const rtl::Reference<VendorBase>& cur = *i;

        if (!sVendor.equals(cur->getVendor()))
            continue;

        javaPluginError err = checkJavaVersionRequirements(
            cur, sMinVersion, sMaxVersion, arExcludeList);

        if (err == javaPluginError::FailedVersion || err == javaPluginError::WrongArch)
            continue;
        else if (err == javaPluginError::WrongVersionFormat)
            return err;

        vecVerifiedInfos.push_back(*i);
    }
    //Now vecVerifiedInfos contains all those JREs which meet the version requirements
    //Transfer them into the array that is passed out.
    parJavaInfo->clear();
    typedef vector<rtl::Reference<VendorBase> >::const_iterator cit;
    for (cit ii = vecVerifiedInfos.begin(); ii != vecVerifiedInfos.end(); ++ii)
    {
        parJavaInfo->push_back(createJavaInfo(*ii));
    }

    return javaPluginError::NONE;
}

javaPluginError jfw_plugin_getJavaInfoByPath(
    OUString const& sPath,
    OUString const& sVendor,
    OUString const& sMinVersion,
    OUString const& sMaxVersion,
    std::vector<OUString> const &arExcludeList,
    std::unique_ptr<JavaInfo> * ppInfo)
{
    assert(ppInfo != nullptr);
    OSL_ASSERT(!sPath.isEmpty());
    if (sPath.isEmpty())
        return javaPluginError::InvalidArg;

    OSL_ASSERT(!sVendor.isEmpty());
    if (sVendor.isEmpty())
        return javaPluginError::InvalidArg;

    rtl::Reference<VendorBase> aVendorInfo = getJREInfoByPath(sPath);
    if (!aVendorInfo.is())
        return javaPluginError::NoJre;

    //Check if the detected JRE matches the version requirements
    if (!sVendor.equals(aVendorInfo->getVendor()))
        return javaPluginError::NoJre;
    javaPluginError errorcode = checkJavaVersionRequirements(
            aVendorInfo, sMinVersion, sMaxVersion, arExcludeList);

    if (errorcode == javaPluginError::NONE)
        *ppInfo = createJavaInfo(aVendorInfo);

    return errorcode;
}

javaPluginError jfw_plugin_getJavaInfoFromJavaHome(
    std::vector<pair<OUString, jfw::VersionInfo>> const& vecVendorInfos,
    std::unique_ptr<JavaInfo> * ppInfo,
    std::vector<rtl::Reference<VendorBase>> & infos)
{
    assert(ppInfo);

    std::vector<rtl::Reference<VendorBase>> infoJavaHome;
    addJavaInfoFromJavaHome(infos, infoJavaHome);

    if (infoJavaHome.empty())
        return javaPluginError::NoJre;
    assert(infoJavaHome.size() == 1);

    //Check if the detected JRE matches the version requirements
    typedef std::vector<pair<OUString, jfw::VersionInfo>>::const_iterator ci_pl;
    for (ci_pl vendorInfo = vecVendorInfos.begin(); vendorInfo != vecVendorInfos.end(); ++vendorInfo)
    {
        const OUString& vendor = vendorInfo->first;
        jfw::VersionInfo versionInfo = vendorInfo->second;

        if (vendor.equals(infoJavaHome[0]->getVendor()))
        {
            javaPluginError errorcode = checkJavaVersionRequirements(
                infoJavaHome[0],
                versionInfo.sMinVersion,
                versionInfo.sMaxVersion,
                versionInfo.vecExcludeVersions);

            if (errorcode == javaPluginError::NONE)
            {
                *ppInfo = createJavaInfo(infoJavaHome[0]);
                return javaPluginError::NONE;
            }
        }
    }

    return javaPluginError::NoJre;
}

javaPluginError jfw_plugin_getJavaInfosFromPath(
    std::vector<std::pair<OUString, jfw::VersionInfo>> const& vecVendorInfos,
    std::vector<std::unique_ptr<JavaInfo>> & javaInfosFromPath,
    std::vector<rtl::Reference<jfw_plugin::VendorBase>> & infos)
{
    // find JREs from PATH
    vector<rtl::Reference<VendorBase>> vecInfosFromPath;
    addJavaInfosFromPath(infos, vecInfosFromPath);

    vector<std::unique_ptr<JavaInfo>> vecVerifiedInfos;

    // copy infos of JREs that meet version requirements to vecVerifiedInfos
    typedef vector<rtl::Reference<VendorBase> >::iterator it;
    for (it i= vecInfosFromPath.begin(); i != vecInfosFromPath.end(); ++i)
    {
        const rtl::Reference<VendorBase>& currentInfo = *i;

        typedef std::vector<pair<OUString, jfw::VersionInfo>>::const_iterator ci_pl;
        for (ci_pl vendorInfo = vecVendorInfos.begin(); vendorInfo != vecVendorInfos.end(); ++vendorInfo)
        {
            const OUString& vendor = vendorInfo->first;
            jfw::VersionInfo const & versionInfo = vendorInfo->second;

            if (vendor.equals(currentInfo->getVendor()))
            {
                javaPluginError errorcode = checkJavaVersionRequirements(
                    currentInfo,
                    versionInfo.sMinVersion,
                    versionInfo.sMaxVersion,
                    versionInfo.vecExcludeVersions);

                if (errorcode == javaPluginError::NONE)
                {
                    vecVerifiedInfos.push_back(createJavaInfo(currentInfo));
                }
            }
        }
    }

    if (vecVerifiedInfos.empty())
        return javaPluginError::NoJre;

    javaInfosFromPath = std::move(vecVerifiedInfos);

    return javaPluginError::NONE;
}


#if defined(_WIN32)

// Load msvcr71.dll using an explicit full path from where it is
// present as bundled with the JRE. In case it is not found where we
// think it should be, do nothing, and just let the implicit loading
// that happens when loading the JVM take care of it.

static void load_msvcr(OUString const & jvm_dll, OUStringLiteral msvcr)
{
    // First check if msvcr71.dll is in the same folder as jvm.dll. It
    // normally isn't, at least up to 1.6.0_22, but who knows if it
    // might be in the future.
    sal_Int32 slash = jvm_dll.lastIndexOf('\\');

    if (slash == -1)
    {
        // Huh, weird path to jvm.dll. Oh well.
        SAL_WARN("jfw", "JVM pathname <" + jvm_dll + "> w/o backslash");
        return;
    }

    if (LoadLibraryW(
            reinterpret_cast<wchar_t const *>(
                OUString(jvm_dll.copy(0, slash+1) + msvcr).getStr())))
        return;

    // Then check if msvcr71.dll is in the parent folder of where
    // jvm.dll is. That is currently (1.6.0_22) as far as I know the
    // normal case.
    slash = jvm_dll.lastIndexOf('\\', slash);

    if (slash == -1)
        return;

    LoadLibraryW(
        reinterpret_cast<wchar_t const *>(
            OUString(jvm_dll.copy(0, slash+1) + msvcr).getStr()));
}

// Check if the jvm DLL imports msvcr71.dll, and in that case try
// loading it explicitly. In case something goes wrong, do nothing,
// and just let the implicit loading try to take care of it.
static void do_msvcr_magic(OUString const &jvm_dll)
{
    struct stat st;

    OUString Module;
    osl::FileBase::RC nError = osl::FileBase::getSystemPathFromFileURL(
        jvm_dll, Module);

    if ( osl::FileBase::E_None != nError )
    {
        SAL_WARN(
            "jfw", "getSystemPathFromFileURL(" << jvm_dll << "): " << +nError);
        return;
    }

    FILE *f = _wfopen(reinterpret_cast<LPCWSTR>(Module.getStr()), L"rb");

    if (!f)
    {
        SAL_WARN("jfw", "_wfopen(" << Module << ") failed");
        return;
    }

    if (fstat(fileno(f), &st) == -1)
    {
        SAL_WARN("jfw", "fstat(" << Module << ") failed");
        fclose(f);
        return;
    }

    PIMAGE_DOS_HEADER dos_hdr = static_cast<PIMAGE_DOS_HEADER>(malloc(st.st_size));

    if (fread(dos_hdr, st.st_size, 1, f) != 1 ||
        memcmp(dos_hdr, "MZ", 2) != 0 ||
        dos_hdr->e_lfanew < 0 ||
        dos_hdr->e_lfanew > (LONG) (st.st_size - sizeof(IMAGE_NT_HEADERS)))
    {
        SAL_WARN("jfw", "analyzing <" << Module << "> failed");
        free(dos_hdr);
        fclose(f);
        return;
    }

    fclose(f);

    IMAGE_NT_HEADERS *nt_hdr = reinterpret_cast<IMAGE_NT_HEADERS *>(reinterpret_cast<char *>(dos_hdr) + dos_hdr->e_lfanew);

    DWORD importsVA = nt_hdr->OptionalHeader
            .DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    // first determine Virtual-to-File-address mapping for the section
    // that contains the import directory
    IMAGE_SECTION_HEADER *sections = IMAGE_FIRST_SECTION(nt_hdr);
    ptrdiff_t VAtoPhys = -1;
    for (int i = 0; i < nt_hdr->FileHeader.NumberOfSections; ++i)
    {
        if (sections->VirtualAddress <= importsVA &&
            importsVA < sections->VirtualAddress + sections->SizeOfRawData)
        {
            VAtoPhys = static_cast<size_t>(sections->PointerToRawData) - static_cast<size_t>(sections->VirtualAddress);
            break;
        }
        ++sections;
    }
    if (-1 == VAtoPhys) // not found?
    {
        SAL_WARN("jfw", "analyzing <" << Module << "> failed");
        free(dos_hdr);
        return;
    }
    IMAGE_IMPORT_DESCRIPTOR *imports =
        reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR *>(reinterpret_cast<char *>(dos_hdr) + importsVA + VAtoPhys);

    while (imports <= reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR *>(reinterpret_cast<char *>(dos_hdr) + st.st_size - sizeof (IMAGE_IMPORT_DESCRIPTOR)) &&
           imports->Name != 0 &&
           imports->Name + VAtoPhys < (DWORD) st.st_size)
    {
        static OUStringLiteral msvcrts[] =
        {
            "msvcr71.dll",
            "msvcr100.dll"
        };
        char const* importName = reinterpret_cast<char *>(dos_hdr) + imports->Name + VAtoPhys;
        sal_Int32 importNameLen = rtl_str_getLength(importName);
        for (size_t i = 0; i < SAL_N_ELEMENTS(msvcrts); ++i)
        {
            if (0 == rtl_str_compareIgnoreAsciiCase_WithLength(
                    importName, importNameLen,
                    msvcrts[i].data, msvcrts[i].size))
            {
                load_msvcr(Module, msvcrts[i]);
                free(dos_hdr);
                return;
            }
        }
        imports++;
    }

    free(dos_hdr);
}

#endif

/** starts a Java Virtual Machine.
    <p>
    The function shall ensure, that the VM does not abort the process
    during instantiation.
    </p>
 */
javaPluginError jfw_plugin_startJavaVirtualMachine(
    const JavaInfo *pInfo,
    const JavaVMOption* arOptions,
    sal_Int32 cOptions,
    JavaVM ** ppVm,
    JNIEnv ** ppEnv)
{
    assert(pInfo != nullptr);
    assert(ppVm != nullptr);
    assert(ppEnv != nullptr);
    // unless guard is volatile the following warning occurs on gcc:
    // warning: variable 't' might be clobbered by `longjmp' or `vfork'
    volatile osl::MutexGuard guard(PluginMutex::get());
    // unless errorcode is volatile the following warning occurs on gcc:
    // warning: variable 'errorcode' might be clobbered by `longjmp' or `vfork'
    volatile javaPluginError errorcode = javaPluginError::NONE;
    //Check if the Vendor (pInfo->sVendor) is supported by this plugin
    if ( ! isVendorSupported(pInfo->sVendor))
        return javaPluginError::WrongVendor;
#ifdef MACOSX
#ifdef USE_JAVA
    try
    {
#endif	// USE_JAVA
    rtl::Reference<VendorBase> aVendorInfo = getJREInfoByPath( OUString( pInfo->sLocation ) );
    if ( !aVendorInfo.is() || aVendorInfo->compareVersions( OUString( pInfo->sVersion ) ) < 0 )
#ifdef USE_JAVA
        return javaPluginError::FailedVersion;
    }
    catch ( MalformedVersionException& )
    {
        return javaPluginError::WrongVersionFormat;
    }
#else	// USE_JAVA
        return javaPluginError::VmCreationFailed;
#endif	// USE_JAVA
#endif
    OUString sRuntimeLib = getRuntimeLib(pInfo->arVendorData);
#ifdef MACOSX
    if ( !JvmfwkUtil_isLoadableJVM( sRuntimeLib ) )
        return javaPluginError::VmCreationFailed;
#endif
    JFW_TRACE2("Using Java runtime library: " << sRuntimeLib);

#ifndef ANDROID
    // On linux we load jvm with RTLD_GLOBAL. This is necessary for debugging, because
    // libjdwp.so need a symbol (fork1) from libjvm which it only gets if the jvm is loaded
    // with RTLD_GLOBAL. On Solaris libjdwp.so is correctly linked with libjvm.so
    osl::Module moduleRt;
#if defined(LINUX)
    if (!moduleRt.load(sRuntimeLib, SAL_LOADMODULE_GLOBAL | SAL_LOADMODULE_NOW))
#elif defined MACOSX
    // Must be SAL_LOADMODULE_GLOBAL when e.g. specifying a
    // -Xrunjdwp:transport=dt_socket,server=y,suspend=n,address=8000 option to
    // JDK 1.8.0_121 at least, as JNI_CreateJavaVM -> Threads::create_vm ->
    // JvmtiExport::post_vm_initialized -> cbEarlyVMInit -> initialize ->
    // util_initialize -> sun.misc.VMSupport.getAgentProperties ->
    // Java_sun_misc_VMSupport_initAgentProperties ->
    // JDK_FindJvmEntry("JVM_INitAgentProperties") ->
    // dlsym(RTLD_DEFAULT, "JVM_INitAgentProperties"):
    if (!moduleRt.load(sRuntimeLib, SAL_LOADMODULE_GLOBAL))
#else
#if defined(_WIN32)
    do_msvcr_magic(sRuntimeLib);
#endif
    if (!moduleRt.load(sRuntimeLib))
#endif
    {
        JFW_ENSURE(false,
                   "[Java framework]sunjavaplugin" SAL_DLLEXTENSION
                      " could not load Java runtime library: \n"
                   + sRuntimeLib + "\n");
        JFW_TRACE0("Could not load Java runtime library: " << sRuntimeLib);
        return javaPluginError::VmCreationFailed;
    }

#if ( defined UNX && !defined MACOSX ) || defined USE_JAVA
    //Setting the JAVA_HOME is needed for awt
    OUString sPathLocation;
    osl::FileBase::getSystemPathFromFileURL(pInfo->sLocation, sPathLocation);
    osl_setEnvironment(OUString("JAVA_HOME").pData, sPathLocation.pData);
#endif	// ( UNX && !MACOSX ) || USE_JAVA

    typedef jint JNICALL JNI_CreateVM_Type(JavaVM **, JNIEnv **, void *);
#if defined USE_JAVA && defined MACOSX
    // Fix bug 1257 by explicitly loading the JVM instead of loading the
    // shared JavaVM library
    OUString sSymbolCreateJava("JNI_CreateJavaVM_Impl");
    OUString sSymbolCreateJava2("JNI_CreateJavaVM");
#else	// USE_JAVA && MACOSX
    OUString sSymbolCreateJava("JNI_CreateJavaVM");
#endif	// USE_JAVA && MACOSX

    JNI_CreateVM_Type * pCreateJavaVM =
        reinterpret_cast<JNI_CreateVM_Type *>(moduleRt.getFunctionSymbol(sSymbolCreateJava));
#if defined USE_JAVA && defined MACOSX
    if (!pCreateJavaVM)
        pCreateJavaVM = reinterpret_cast<JNI_CreateVM_Type *>(moduleRt.getFunctionSymbol(sSymbolCreateJava2));
#endif	// USE_JAVA && MACOSX
    if (!pCreateJavaVM)
    {
        OSL_ASSERT(false);
        OString sLib = OUStringToOString(
            sRuntimeLib, osl_getThreadTextEncoding());
        OString sSymbol = OUStringToOString(
            sSymbolCreateJava, osl_getThreadTextEncoding());
        fprintf(stderr,"[Java framework]sunjavaplugin" SAL_DLLEXTENSION
                "Java runtime library: %s does not export symbol %s !\n",
                sLib.getStr(), sSymbol.getStr());
        return javaPluginError::VmCreationFailed;
    }
    moduleRt.release();

    // Valgrind typically emits many false errors when executing JIT'ed JVM
    // code, so force the JVM into interpreted mode:
    bool addForceInterpreted = FORCE_INTERPRETED > 0;

    // Some testing with Java 1.4 showed that JavaVMOption.optionString has to
    // be encoded with the system encoding (i.e., osl_getThreadTextEncoding):
    JavaVMInitArgs vm_args;

    struct Option {
        Option(OString const & theOptionString, void * theExtraInfo):
            optionString(theOptionString), extraInfo(theExtraInfo)
        {}

        OString optionString;
        void * extraInfo;
    };
    std::vector<Option> options;

    // We set an abort handler which is called when the VM calls _exit during
    // JNI_CreateJavaVM. This happens when the LD_LIBRARY_PATH does not contain
    // all some directories of the Java installation. This is necessary for
    // all versions below 1.5.1
    options.push_back(Option("abort", reinterpret_cast<void*>(abort_handler)));
    bool hasStackSize = false;
    for (int i = 0; i < cOptions; i++)
    {
        OString opt(arOptions[i].optionString);
#ifdef UNX
        // Until java 1.5 we need to put a plugin.jar or javaplugin.jar (<1.4.2)
        // in the class path in order to have applet support:
        if (opt.startsWith("-Djava.class.path="))
        {
            OString sAddPath = getPluginJarPath(pInfo->sVendor, pInfo->sLocation,pInfo->sVersion);
            if (!sAddPath.isEmpty())
                opt += OString(SAL_PATHSEPARATOR) + sAddPath;
        }
#endif
        if (opt == "-Xint") {
            addForceInterpreted = false;
        }
        if (opt.startsWith("-Xss")) {
            hasStackSize = true;
        }
#ifdef USE_JAVA
        // Fix failure to load Java 10.x in dbaccess unit tests by not adding
        // empty options
        if (opt.getLength())
#endif	// USE_JAVA
        options.push_back(Option(opt, arOptions[i].extraInfo));
    }
    if (addForceInterpreted) {
        options.push_back(Option("-Xint", nullptr));
    }
    if (!hasStackSize) {
#if defined LINUX && (defined X86 || defined X86_64)
        // At least OpenJDK 1.8.0's os::workaround_expand_exec_shield_cs_limit
        // (hotspot/src/os_cpu/linux_x86/vm/os_linux_x86.cpp) can mmap an rwx
        // page into the area that the main stack can grow down to according to
        // "ulimit -s", as os::init_2's (hotspot/src/os/linux/vm/os_linux.cpp)
        // call to
        //
        //   Linux::capture_initial_stack(JavaThread::stack_size_at_create());
        //
        // caps _initial_thread_stack_size at threadStackSizeInBytes ,i.e.,
        // -Xss, which appears to default to only 327680, whereas "ulimit -s"
        // defaults to 8192 * 1024 at least on Fedora 20; so attempt to pass in
        // a useful -Xss argument:
        rlimit l;
        if (getrlimit(RLIMIT_STACK, &l) == 0) {
            if (l.rlim_cur == RLIM_INFINITY) {
                SAL_INFO("jfw", "RLIMIT_STACK RLIM_INFINITY -> 8192K");
                l.rlim_cur = 8192 * 1024;
            } else if (l.rlim_cur > 512 * 1024 * 1024) {
                SAL_INFO(
                    "jfw", "huge RLIMIT_STACK " << l.rlim_cur << " -> 8192K");
                l.rlim_cur = 8192 * 1024;
            }
            options.push_back(
                Option("-Xss" + OString::number(l.rlim_cur), nullptr));
        } else {
            int e = errno;
            SAL_WARN("jfw", "getrlimit(RLIMIT_STACK) failed with errno " << e);
        }
#endif
    }

#ifdef USE_JAVA
    OString aPathDelimiter( SAL_PATHDELIMITER );
    OString aPathSeparator( SAL_PATHSEPARATOR );

    // Limit the directories that extensions can be loaded from to prevent
    // random JVM crashing. Fix failure to load Java 10.x by only adding
    // directories that actually exist.
    OString aExtProp( "-Djava.ext.dirs=" );
    OUString aExtPath( sPathLocation );
    aExtPath += OStringToOUString( aPathDelimiter + "lib" + aPathDelimiter + "ext", osl_getThreadTextEncoding() );
    OUString aExtURL;
    if ( osl_getFileURLFromSystemPath( aExtPath.pData, &aExtURL.pData ) == osl_File_E_None )
    {
        DirectoryItem aDirItem;
        if ( DirectoryItem::get( aExtURL, aDirItem ) == File::E_None )
            aExtProp += OUStringToOString( aExtPath, osl_getThreadTextEncoding() );
    }
    options.push_back( Option( aExtProp, nullptr ) );

    // Set the library path to include the executable path but none of the
    // extensions
    OString aLibPath( "-Djava.library.path=" );
    sal_Int32 lastIndex = sPathLocation.lastIndexOf( SAL_PATHDELIMITER );
    if ( lastIndex > 0 )
    {
        OString aJavaLibPath = OUStringToOString( sPathLocation.copy( 0, lastIndex ), osl_getThreadTextEncoding() );
        aJavaLibPath += aPathDelimiter;
#ifdef MACOSX
        aJavaLibPath += "lib";
#else	// MACOSX
        aJavaLibPath += "bin";
#endif	// MACOSX
        aLibPath += aPathSeparator;
        aLibPath += aJavaLibPath;
    }
#ifdef MACOSX
    OString aEnvLibPath( getenv( "DYLD_LIBRARY_PATH" ) );
    if  ( aEnvLibPath.getLength() )
    {
        aLibPath += aPathSeparator;
        aLibPath += aEnvLibPath;
    }
    OString aEnvFallbackLibPath( getenv( "DYLD_FALLBACK_LIBRARY_PATH" ) );
    if ( aEnvFallbackLibPath.getLength() )
    {
        aLibPath += aPathSeparator;
        aLibPath += aEnvFallbackLibPath;
    }
#else	// MACOSX
    OString aEnvPath( getenv( "PATH" ) );
    if  ( aEnvPath.getLength() )
    {
        aLibPath += aPathSeparator;
        aLibPath += aEnvPath;
    }
#endif	// MACOSX
    options.push_back( Option( aLibPath, nullptr ) );

    // Set miscellaneous optimizations for the JVM
    options.push_back( Option( "-Xrs", nullptr ) );

    size_t nUserMem = 256;
    OString aMemMax( "-Xmx" );
    aMemMax += OString::number( static_cast< sal_Int32 >( nUserMem ) ) + "m";
    options.push_back( Option( aMemMax, nullptr ) );

#ifdef MACOSX
    // Enable Java AWT as the hanging caused by Oracle's Java AWT classes has
    // been fixed in vcl/source/app/svmainhook_cocoa.mm
    options.push_back( Option( "-Djava.awt.headless=false", nullptr ) );
#endif	// MACOSX
#endif	// USE_JAVA

    std::unique_ptr<JavaVMOption[]> sarOptions(new JavaVMOption[options.size()]);
    for (std::vector<Option>::size_type i = 0; i != options.size(); ++i) {
        SAL_INFO(
            "jfw",
            "VM option \"" << options[i].optionString << "\" "
                << options[i].extraInfo);
        sarOptions[i].optionString = const_cast<char *>(
            options[i].optionString.getStr());
        sarOptions[i].extraInfo = options[i].extraInfo;
    }

#ifdef MACOSX
    vm_args.version= JNI_VERSION_1_4; // issue 88987
#else
    vm_args.version= JNI_VERSION_1_2;
#endif
    vm_args.options= sarOptions.get();
    vm_args.nOptions= options.size(); //TODO overflow
    vm_args.ignoreUnrecognized= JNI_TRUE;

    /* We set a global flag which is used by the abort handler in order to
       determine whether it is  should use longjmp to get back into this function.
       That is, the abort handler determines if it is on the same stack as this function
       and then jumps back into this function.
    */
    g_bInGetJavaVM = 1;
    jint err;
    JavaVM * pJavaVM = nullptr;
    memset( jmp_jvm_abort, 0, sizeof(jmp_jvm_abort));
    int jmpval= setjmp( jmp_jvm_abort );
    /* If jmpval is not "0" then this point was reached by a longjmp in the
       abort_handler, which was called indirectly by JNI_CreateVM.
    */
    if( jmpval == 0)
    {
        //returns negative number on failure
        err= pCreateJavaVM(&pJavaVM, ppEnv, &vm_args);
        g_bInGetJavaVM = 0;
    }
    else
        // set err to a positive number, so as or recognize that an abort (longjmp)
        //occurred
        err= 1;

    if(err != 0)
    {
        if( err < 0)
        {
            fprintf(stderr,"[Java framework] sunjavaplugin" SAL_DLLEXTENSION
                    "Can not create Java Virtual Machine\n");
            errorcode = javaPluginError::VmCreationFailed;
        }
        else if( err > 0)
        {
            fprintf(stderr,"[Java framework] sunjavaplugin" SAL_DLLEXTENSION
                    "Can not create JavaVirtualMachine, abort handler was called.\n");
            errorcode = javaPluginError::VmCreationFailed;
        }
    }
    else
    {
        *ppVm = pJavaVM;
        JFW_TRACE2("JVM created");
    }
#else
    (void) arOptions;
    (void) cOptions;
    (void) ppEnv;
    // On Android we always have a Java VM as we only expect this code
    // to be run in an Android app anyway.
    *ppVm = lo_get_javavm();
    fprintf(stderr, "lo_get_javavm returns %p", *ppVm);
#endif

   return errorcode;
}

javaPluginError jfw_plugin_existJRE(const JavaInfo *pInfo, bool *exist)
{
    assert(pInfo != nullptr);
    assert(exist != nullptr);

    javaPluginError ret = javaPluginError::NONE;
    OUString sLocation(pInfo->sLocation);

    if (sLocation.isEmpty())
        return javaPluginError::InvalidArg;
    ::osl::DirectoryItem item;
    ::osl::File::RC rc_item = ::osl::DirectoryItem::get(sLocation, item);
    if (::osl::File::E_None == rc_item)
    {
        *exist = true;
    }
    else if (::osl::File::E_NOENT == rc_item)
    {
        *exist = false;
    }
    else
    {
        ret = javaPluginError::Error;
    }
    //We can have the situation that the JavaVM runtime library is not
    //contained within JAVA_HOME. Then the check for JAVA_HOME would return
    //true although the runtime library may not be loadable.
    //Or the JAVA_HOME directory of a deinstalled JRE left behind.
    if (ret == javaPluginError::NONE && *exist)
    {
        OUString sRuntimeLib = getRuntimeLib(pInfo->arVendorData);
        JFW_TRACE2("Checking existence of Java runtime library");

        ::osl::DirectoryItem itemRt;
        ::osl::File::RC rc_itemRt = ::osl::DirectoryItem::get(sRuntimeLib, itemRt);
        if (::osl::File::E_None == rc_itemRt)
        {
            *exist = true;
            JFW_TRACE2("Java runtime library exist: " << sRuntimeLib);

        }
        else if (::osl::File::E_NOENT == rc_itemRt)
        {
            *exist = false;
            JFW_TRACE2("Java runtime library does not exist: " << sRuntimeLib);
        }
        else
        {
            ret = javaPluginError::Error;
            JFW_TRACE2("Error while looking for Java runtime library: " << sRuntimeLib);
        }
    }
    return ret;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
