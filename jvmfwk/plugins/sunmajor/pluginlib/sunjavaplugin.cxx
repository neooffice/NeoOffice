/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU General Public License Version 2.1.
 *
 *
 *    GNU General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 *    Modified December 2005 by Patrick Luby. NeoOffice is distributed under
 *    GPL only under modification term 3 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_jvmfwk.hxx"
#if OSL_DEBUG_LEVEL > 0
#include <stdio.h>
#endif

#include "boost/scoped_array.hpp"
#include "osl/diagnose.h"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "osl/module.hxx"
#include "osl/mutex.hxx"
#include "osl/thread.hxx"
#include "osl/file.hxx"
#include "rtl/instance.hxx"
#include "osl/getglobalmutex.hxx"
#include <setjmp.h>
#include <signal.h>
#include <stack>

#include "jni.h"
#include "rtl/byteseq.hxx"
#include "jvmfwk/vendorplugin.h"
#include "util.hxx"
#include "sunversion.hxx"
#include "vendorlist.hxx"
#include "diagnostics.h"

#ifdef USE_JAVA
#include "osl/process.h"
#include "rtl/strbuf.hxx"
#include <sys/sysctl.h>
#include <unistd.h>
#endif	// USE_JAVA

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
#define SUN_MICRO "Sun Microsystems Inc."

using namespace osl;
using namespace rtl;
using namespace std;
using namespace jfw_plugin;

namespace {

struct Init
{
    osl::Mutex * operator()()
        {
            static osl::Mutex aInstance;
            return &aInstance;
        }
};
osl::Mutex * getPluginMutex()
{
    return rtl_Instance< osl::Mutex, Init, ::osl::MutexGuard,
        ::osl::GetGlobalMutex >::create(
            Init(), ::osl::GetGlobalMutex());
}

#if defined UNX
OString getPluginJarPath(
    const OUString & sVendor,
    const OUString& sLocation,
    const OUString& sVersion) 
{
    OString ret;
    OUString sName1(RTL_CONSTASCII_USTRINGPARAM("javaplugin.jar"));
    OUString sName2(RTL_CONSTASCII_USTRINGPARAM("plugin.jar"));
    OUString sPath;
    if (sVendor.equals(OUString(RTL_CONSTASCII_USTRINGPARAM(SUN_MICRO))))
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
        if (sName.getLength())
        {
            sName = sLocation + OUSTR("/lib/") + sName;
            OSL_VERIFY(
                osl_getSystemPathFromFileURL(sName.pData, & sPath.pData)
                == osl_File_E_None);
        }
    }
    else
    {
        char sep[] =  {SAL_PATHSEPARATOR, 0};
        OUString sName(sLocation + OUSTR("/lib/") + sName1);
        OUString sPath1;
        OUString sPath2;
        if (osl_getSystemPathFromFileURL(sName.pData, & sPath1.pData)
            == osl_File_E_None)
        {
            sName = sLocation + OUSTR("/lib/") + sName2;
            if (osl_getSystemPathFromFileURL(sName.pData, & sPath2.pData)
                == osl_File_E_None)
            {
                sPath = sPath1 + OUString::createFromAscii(sep) + sPath2;
            }   
        }
        OSL_ASSERT(sPath.getLength());
    }
    ret = rtl::OUStringToOString(sPath, osl_getThreadTextEncoding());
    
    return ret;
}
#endif // UNX


JavaInfo* createJavaInfo(const rtl::Reference<VendorBase> & info)
{
    JavaInfo* pInfo = (JavaInfo*) rtl_allocateMemory(sizeof(JavaInfo));
    if (pInfo == NULL)
        return NULL;
    rtl::OUString sVendor = info->getVendor();
    pInfo->sVendor = sVendor.pData;
    rtl_uString_acquire(sVendor.pData);
    rtl::OUString sHome = info->getHome();
    pInfo->sLocation = sHome.pData;
    rtl_uString_acquire(pInfo->sLocation);
    rtl::OUString sVersion = info->getVersion();
    pInfo->sVersion = sVersion.pData;
    rtl_uString_acquire(pInfo->sVersion);
    pInfo->nFeatures = info->supportsAccessibility() ? 1 : 0;
    pInfo->nRequirements = info->needsRestart() ? JFW_REQUIRE_NEEDRESTART : 0;
    rtl::OUStringBuffer buf(1024);
    buf.append(info->getRuntimeLibrary());
    if (info->getLibraryPaths().getLength() > 0)
    {
        buf.appendAscii("\n");
        buf.append(info->getLibraryPaths());
        buf.appendAscii("\n");
    }
    
    rtl::OUString sVendorData = buf.makeStringAndClear();
    rtl::ByteSequence byteSeq( (sal_Int8*) sVendorData.pData->buffer,
                               sVendorData.getLength() * sizeof(sal_Unicode));
    pInfo->arVendorData = byteSeq.get();
    rtl_byte_sequence_acquire(pInfo->arVendorData);

    return pInfo;
}   

rtl::OUString getRuntimeLib(const rtl::ByteSequence & data)
{
    const sal_Unicode* chars = (sal_Unicode*) data.getConstArray();
    sal_Int32 len = data.getLength();
    rtl::OUString sData(chars, len / 2);
    //the runtime lib is on the first line
    sal_Int32 index = 0;
    rtl::OUString aToken = sData.getToken( 0, '\n', index);
    
    return aToken;
}

jmp_buf jmp_jvm_abort;
sig_atomic_t g_bInGetJavaVM = 0;

void abort_handler()
{
    // If we are within JNI_CreateJavaVM then we jump back into getJavaVM
    if( g_bInGetJavaVM != 0 )
    {
        fprintf( stderr, "JavaVM: JNI_CreateJavaVM called _exit, caught by abort_handler in javavm.cxx\n");
        longjmp( jmp_jvm_abort, 0);
    }
}

}

extern "C"
javaPluginError jfw_plugin_getAllJavaInfos(
    rtl_uString *sVendor,
    rtl_uString *sMinVersion,
    rtl_uString *sMaxVersion,
    rtl_uString  * *arExcludeList,
    sal_Int32  nLenList,
    JavaInfo*** parJavaInfo,
    sal_Int32 *nLenInfoList)
{
    OSL_ASSERT(sVendor);
    OSL_ASSERT(sMinVersion);
    OSL_ASSERT(sMaxVersion);
    OSL_ASSERT(parJavaInfo);
    OSL_ASSERT(parJavaInfo);
    OSL_ASSERT(nLenInfoList);
    if (!sVendor || !sMinVersion || !sMaxVersion || !parJavaInfo || !nLenInfoList)
        return JFW_PLUGIN_E_INVALID_ARG;

    //nLenlist contains the number of element in arExcludeList.
    //If no exclude list is provided then nLenList must be 0
    OSL_ASSERT( ! (arExcludeList == NULL && nLenList > 0));
    if (arExcludeList == NULL && nLenList > 0)
        return JFW_PLUGIN_E_INVALID_ARG;

    OUString ouVendor(sVendor);
    OUString ouMinVer(sMinVersion);
    OUString ouMaxVer(sMaxVersion);

    OSL_ASSERT(ouVendor.getLength() > 0);
    if (ouVendor.getLength() == 0)
        return JFW_PLUGIN_E_INVALID_ARG;

    JavaInfo** arInfo = NULL;

    //Find all JREs
    vector<rtl::Reference<VendorBase> > vecInfos =
        getAllJREInfos();
    vector<rtl::Reference<VendorBase> > vecVerifiedInfos;

    typedef vector<rtl::Reference<VendorBase> >::iterator it;    
    for (it i= vecInfos.begin(); i != vecInfos.end(); i++)
    {
        const rtl::Reference<VendorBase>& cur = *i;
        
        if (ouVendor.equals(cur->getVendor()) == sal_False)
            continue;
        
        if (ouMinVer.getLength() > 0)
        {
            try
            {
                if (cur->compareVersions(sMinVersion) == -1)
                    continue;
            }
            catch (MalformedVersionException&)
            {
                //The minVersion was not recognized as valid for this vendor.
                JFW_ENSURE(
                    0,OUSTR("[Java framework]sunjavaplugin does not know version: ")
                    + ouMinVer + OUSTR(" for vendor: ") + cur->getVendor()
                    + OUSTR(" .Check minimum Version.") );                    
                return JFW_PLUGIN_E_WRONG_VERSION_FORMAT;
            }
        }
        
        if (ouMaxVer.getLength() > 0)
        {
            try
            {
                if (cur->compareVersions(sMaxVersion) == 1)
                    continue;
            }
            catch (MalformedVersionException&)
            {
                //The maxVersion was not recognized as valid for this vendor.
                JFW_ENSURE(
                    0,OUSTR("[Java framework]sunjavaplugin does not know version: ")
                    + ouMaxVer + OUSTR(" for vendor: ") + cur->getVendor()
                    + OUSTR(" .Check maximum Version.") );
                return JFW_PLUGIN_E_WRONG_VERSION_FORMAT;
            }
        }
        
        if (arExcludeList > 0)
        {
            bool bExclude = false;
            for (int j = 0; j < nLenList; j++)
            {
                rtl::OUString sExVer(arExcludeList[j]);
                try
                {
                    if (cur->compareVersions(sExVer) == 0)
                    {
                        bExclude = true;
                        break;
                    }
                }
                catch (MalformedVersionException&)
                {
                    //The excluded version was not recognized as valid for this vendor.
                    JFW_ENSURE(
                        0,OUSTR("[Java framework]sunjavaplugin does not know version: ")
                        + sExVer + OUSTR(" for vendor: ") + cur->getVendor()
                        + OUSTR(" .Check excluded versions.") );
                    return JFW_PLUGIN_E_WRONG_VERSION_FORMAT;
                }
            }
            if (bExclude == true)
                continue;
        }
        vecVerifiedInfos.push_back(*i);
    }
    //Now vecVerifiedInfos contains all those JREs which meet the version requirements
    //Transfer them into the array that is passed out.
    arInfo = (JavaInfo**) rtl_allocateMemory(vecVerifiedInfos.size() * sizeof (JavaInfo*));
    int j = 0;
    typedef vector<rtl::Reference<VendorBase> >::const_iterator cit;
    for (cit ii = vecVerifiedInfos.begin(); ii != vecVerifiedInfos.end(); ii++, j++)
    {
        arInfo[j] = createJavaInfo(*ii);
    }
    *nLenInfoList = vecVerifiedInfos.size();


    *parJavaInfo = arInfo;
    return JFW_PLUGIN_E_NONE;
}

extern "C"
javaPluginError jfw_plugin_getJavaInfoByPath(
    rtl_uString *path,
    rtl_uString *sVendor,
    rtl_uString *sMinVersion,
    rtl_uString *sMaxVersion,
    rtl_uString  *  *arExcludeList,
    sal_Int32  nLenList,
    JavaInfo ** ppInfo)
{
    javaPluginError errcode = JFW_PLUGIN_E_NONE;
    
    OSL_ASSERT(path);
    OSL_ASSERT(sVendor);
    OSL_ASSERT(sMinVersion);
    OSL_ASSERT(sMaxVersion);
    if (!path || !sVendor || !sMinVersion || !sMaxVersion || !ppInfo)
        return JFW_PLUGIN_E_INVALID_ARG;
    OUString ouPath(path);
    OSL_ASSERT(ouPath.getLength() > 0);
    if (ouPath.getLength() == 0)
        return JFW_PLUGIN_E_INVALID_ARG;

    //nLenlist contains the number of element in arExcludeList.
    //If no exclude list is provided then nLenList must be 0
    OSL_ASSERT( ! (arExcludeList == NULL && nLenList > 0));
    if (arExcludeList == NULL && nLenList > 0)
        return JFW_PLUGIN_E_INVALID_ARG;
    
    OUString ouVendor(sVendor);
    OUString ouMinVer(sMinVersion);
    OUString ouMaxVer(sMaxVersion);

    OSL_ASSERT(ouVendor.getLength() > 0);
    if (ouVendor.getLength() == 0)
        return JFW_PLUGIN_E_INVALID_ARG;
        
    rtl::Reference<VendorBase> aVendorInfo = getJREInfoByPath(ouPath);
    if (aVendorInfo.is() == sal_False)
        return JFW_PLUGIN_E_NO_JRE;
    
    //Check if the detected JRE matches the version requirements
    if (ouVendor.equals(aVendorInfo->getVendor()) == sal_False)
        return JFW_PLUGIN_E_NO_JRE;
    
    if (ouMinVer.getLength() > 0)
    {
        int nRes = 0;
        try
        {
            nRes = aVendorInfo->compareVersions(ouMinVer);
        }
        catch (MalformedVersionException&)
        {
            //The minVersion was not recognized as valid for this vendor.
            JFW_ENSURE(
                0,OUSTR("[Java framework]sunjavaplugin does not know version: ")
                + ouMinVer + OUSTR(" for vendor: ") + aVendorInfo->getVendor()
                + OUSTR(" .Check minimum Version.") );
            return JFW_PLUGIN_E_WRONG_VERSION_FORMAT;
        }
        if (nRes < 0)
            return JFW_PLUGIN_E_FAILED_VERSION;
    }
    
    if (ouMaxVer.getLength() > 0)
    {
        int nRes = 0;
        try
        {
            nRes = aVendorInfo->compareVersions(ouMaxVer);
        }
        catch (MalformedVersionException&)
        {
            //The maxVersion was not recognized as valid for this vendor.
            JFW_ENSURE(
                0,OUSTR("[Java framework]sunjavaplugin does not know version: ")
                + ouMaxVer + OUSTR(" for vendor: ") + aVendorInfo->getVendor()
                + OUSTR(" .Check maximum Version.") );
            return JFW_PLUGIN_E_WRONG_VERSION_FORMAT;
        }
        if (nRes > 0)
            return JFW_PLUGIN_E_FAILED_VERSION;
    }
    
    if (arExcludeList > 0)
    {
        for (int i = 0; i < nLenList; i++)
        {
            rtl::OUString sExVer(arExcludeList[i]);
            int nRes = 0;
            try
            {
                nRes = aVendorInfo->compareVersions(sExVer);
            }
            catch (MalformedVersionException&)
            {
                //The excluded version was not recognized as valid for this vendor.
                JFW_ENSURE(
                    0,OUSTR("[Java framework]sunjavaplugin does not know version: ")
                    + sExVer + OUSTR(" for vendor: ") + aVendorInfo->getVendor()
                    + OUSTR(" .Check excluded versions.") );
                return JFW_PLUGIN_E_WRONG_VERSION_FORMAT;
            }
            if (nRes == 0)
                return JFW_PLUGIN_E_FAILED_VERSION;
        }
    }
    *ppInfo = createJavaInfo(aVendorInfo);

    return errcode;
}

/** starts a Java Virtual Machine.
    <p>
    The function shall ensure, that the VM does not abort the process
    during instantiation.
    </p>
 */
extern "C"
javaPluginError jfw_plugin_startJavaVirtualMachine(
    const JavaInfo *pInfo, 
    const JavaVMOption* arOptions,
    sal_Int32 cOptions,
    JavaVM ** ppVm,
    JNIEnv ** ppEnv)
{
    osl::MutexGuard guard(getPluginMutex());
    // unless errcode is volatile the following warning occurs on gcc:
    // warning: variable 'errcode' might be clobbered by `longjmp' or `vfork'
    volatile javaPluginError errcode = JFW_PLUGIN_E_NONE;
    if ( pInfo == NULL || ppVm == NULL || ppEnv == NULL)
        return JFW_PLUGIN_E_INVALID_ARG;
    //Check if the Vendor (pInfo->sVendor) is supported by this plugin
    if ( ! isVendorSupported(pInfo->sVendor))
        return JFW_PLUGIN_E_WRONG_VENDOR;
    rtl::OUString sRuntimeLib = getRuntimeLib(pInfo->arVendorData);
    JFW_TRACE2(OUSTR("[Java framework] Using Java runtime library: ")
              + sRuntimeLib + OUSTR(".\n"));
    // On linux we load jvm with RTLD_GLOBAL. This is necessary for debugging, because
    // libjdwp.so need a symbol (fork1) from libjvm which it only gets if the jvm is loaded
    // witd RTLD_GLOBAL. On Solaris libjdwp.so is correctly linked with libjvm.so
    oslModule moduleRt = 0;
#if defined(LINUX)
    if ((moduleRt = osl_loadModule(sRuntimeLib.pData,
                                   SAL_LOADMODULE_GLOBAL | SAL_LOADMODULE_NOW)) == 0 )
#else
    if ((moduleRt = osl_loadModule(sRuntimeLib.pData, SAL_LOADMODULE_DEFAULT)) == 0)
#endif
     {
         JFW_ENSURE(0, OUSTR("[Java framework]sunjavaplugin" SAL_DLLEXTENSION
                             " could not load Java runtime library: \n")
                    + sRuntimeLib + OUSTR("."));
         JFW_TRACE0(OUSTR("[Java framework]sunjavaplugin" SAL_DLLEXTENSION
                             " could not load Java runtime library: \n")
                    + sRuntimeLib +  OUSTR("."));
         return JFW_PLUGIN_E_VM_CREATION_FAILED;
     }

#ifdef UNX
    //Setting the JAVA_HOME is needed for awt
    rtl::OUString javaHome(RTL_CONSTASCII_USTRINGPARAM("JAVA_HOME="));
    rtl::OUString sPathLocation;
    osl_getSystemPathFromFileURL(pInfo->sLocation, & sPathLocation.pData);
    javaHome += sPathLocation;
    rtl::OString osJavaHome = rtl::OUStringToOString(
        javaHome, osl_getThreadTextEncoding());
    putenv(strdup(osJavaHome.getStr()));
#endif

    typedef jint JNICALL JNI_InitArgs_Type(void *);
    typedef jint JNICALL JNI_CreateVM_Type(JavaVM **, JNIEnv **, void *);
    rtl::OUString sSymbolCreateJava(
#ifdef USE_JAVA
            // Fix bug 1257 by explicitly loading the JVM instead of loading the
            // shared JavaVM library
            RTL_CONSTASCII_USTRINGPARAM("JNI_CreateJavaVM_Impl"));
#else	// USE_JAVA
            RTL_CONSTASCII_USTRINGPARAM("JNI_CreateJavaVM"));
#endif	// USE_JAVA
        
    JNI_CreateVM_Type * pCreateJavaVM = (JNI_CreateVM_Type *) osl_getFunctionSymbol(
        moduleRt, sSymbolCreateJava.pData);
    if (!pCreateJavaVM)
    {
        OSL_ASSERT(0);
        rtl::OString sLib = rtl::OUStringToOString(
            sRuntimeLib, osl_getThreadTextEncoding());
        rtl::OString sSymbol = rtl::OUStringToOString(
            sSymbolCreateJava, osl_getThreadTextEncoding());
        fprintf(stderr,"[Java framework]sunjavaplugin"SAL_DLLEXTENSION
                "Java runtime library: %s does not export symbol %s !\n",
                sLib.getStr(), sSymbol.getStr());
        return JFW_PLUGIN_E_VM_CREATION_FAILED;
    }

    // The office sets a signal handler at startup. That causes a crash
    // with java 1.3 under Solaris. To make it work, we set back the
    // handler
#ifdef UNX
    struct sigaction act;
    act.sa_handler=SIG_DFL;
    act.sa_flags= 0;
    sigaction( SIGSEGV, &act, NULL);
    sigaction( SIGPIPE, &act, NULL);
    sigaction( SIGBUS, &act, NULL);
    sigaction( SIGILL, &act, NULL);
    sigaction( SIGFPE, &act, NULL);
#endif

    // Some testing with Java 1.4 showed that JavaVMOption.optionString has to
    // be encoded with the system encoding (i.e., osl_getThreadTextEncoding):
    JavaVMInitArgs vm_args;

    boost::scoped_array<JavaVMOption> sarOptions(
#ifdef USE_JAVA
        new JavaVMOption[cOptions + 9]);
#else	// USE_JAVA
        new JavaVMOption[cOptions + 1]);
#endif	// USE_JAVA
    JavaVMOption * options = sarOptions.get();
    
    // We set an abort handler which is called when the VM calls _exit during
    // JNI_CreateJavaVM. This happens when the LD_LIBRARY_PATH does not contain
    // all some directories of the Java installation. This is necessary for
    // all versions below 1.5.1
    options[0].optionString= (char *) "abort";
    options[0].extraInfo= (void* )(sal_IntPtr)abort_handler;
    rtl::OString sClassPathProp("-Djava.class.path=");
    rtl::OString sClassPathOption;
#ifdef USE_JAVA
    int i = 0;
    for (; i < cOptions; i++)
#else	// USE_JAVA
    for (int i = 0; i < cOptions; i++)
#endif	// USE_JAVA
    {
#ifdef UNX
    // Until java 1.5 we need to put a plugin.jar or javaplugin.jar (<1.4.2)
    // in the class path in order to have applet support.
        rtl::OString sClassPath = arOptions[i].optionString;
        if (sClassPath.match(sClassPathProp, 0) == sal_True)
        {
            char sep[] =  {SAL_PATHSEPARATOR, 0};
            OString sAddPath = getPluginJarPath(pInfo->sVendor, pInfo->sLocation,pInfo->sVersion);
            if (sAddPath.getLength())
                sClassPathOption = sClassPath + rtl::OString(sep) + sAddPath;
            else
                sClassPathOption = sClassPath;

#ifdef USE_JAVA
            // Add JREProperties.class to the classpath
            rtl::OUString aExe;
            osl_getExecutableFile( &aExe.pData );
            rtl::OUString aExeSysPath;
            if ( aExe.getLength() && osl_getSystemPathFromFileURL( aExe.pData, &aExeSysPath.pData ) == osl_File_E_None )
            {
                rtl::OString aJREPropsSysPath( aExeSysPath, aExeSysPath.lastIndexOf('/'), RTL_TEXTENCODING_UTF8 );
                sClassPathOption = sClassPathOption + rtl::OString(sep) + aJREPropsSysPath;
            }
#endif	// USE_JAVA

            options[i+1].optionString = (char *) sClassPathOption.getStr();
            options[i+1].extraInfo = arOptions[i].extraInfo;
        }
        else
        {
#endif        
            options[i+1].optionString = arOptions[i].optionString;
            options[i+1].extraInfo = arOptions[i].extraInfo;
#ifdef UNX
        }
#endif
#if OSL_DEBUG_LEVEL >= 2
        JFW_TRACE2(OString("VM option: ") + OString(options[i+1].optionString) +
                   OString("\n"));
#endif
    }

#ifdef USE_JAVA
    // Limit the directories that extensions can be loaded from to prevent
    // random JVM crashing
    rtl::OString aExtPath( "-Djava.ext.dirs=" );
    aExtPath += rtl::OUStringToOString( sPathLocation, osl_getThreadTextEncoding() );
    aExtPath += rtl::OString( "/lib/ext" );
    options[i+1].optionString = (char *)aExtPath.getStr();
    options[i+1].extraInfo = NULL;

    // Set the endorsed directory to use the JVM's XML parser
    rtl::OString aBootPath( "-Xbootclasspath/p:" );
    rtl::OUString aExe;
    osl_getExecutableFile( &aExe.pData );
    rtl::OUString aExeSysPath;
    if ( aExe.getLength() && osl_getSystemPathFromFileURL( aExe.pData, &aExeSysPath.pData ) == osl_File_E_None )
    {
        rtl::OString aDirPath = rtl::OString( aExeSysPath, aExeSysPath.lastIndexOf('/'), RTL_TEXTENCODING_UTF8 );
        aDirPath += rtl::OString( "/classes/" );
        aBootPath += aDirPath;
        aBootPath += rtl::OString( "serializer.jar:" );
        aBootPath += aDirPath;
        aBootPath += rtl::OString( "xalan.jar:" );
        aBootPath += aDirPath;
        aBootPath += rtl::OString( "xercesImpl.jar:" );
        aBootPath += aDirPath;
        aBootPath += rtl::OString( "xml-apis.jar" );
    }
    options[i+2].optionString = (char *)aBootPath.getStr();
    options[i+2].extraInfo = NULL;

    rtl::OString aLibPath( "-Djava.library.path=/usr/lib/java" );
    rtl::OUString aJavaLibPath( pInfo->sLocation );
    aJavaLibPath = rtl::OUString( aJavaLibPath, aJavaLibPath.lastIndexOf('/') );
    aJavaLibPath += OUString::createFromAscii( "/Libraries" );
    rtl::OUString aJavaLibSysPath;
    if ( osl_getSystemPathFromFileURL( aJavaLibPath.pData, &aJavaLibSysPath.pData ) == osl_File_E_None )
    {
        aLibPath += ":";
        aLibPath += rtl::OUStringToOString( aJavaLibSysPath, RTL_TEXTENCODING_UTF8 );
    }
    rtl::OString aEnvLibPath( getenv( "DYLD_LIBRARY_PATH" ) );
    if  ( aEnvLibPath.getLength() )
    {
        aLibPath += ":";
        aLibPath += aEnvLibPath;
    }
    rtl::OString aEnvFallbackLibPath( getenv( "DYLD_FALLBACK_LIBRARY_PATH" ) );
    if ( aEnvFallbackLibPath.getLength() )
    {
        aLibPath += ":";
        aLibPath += aEnvFallbackLibPath;
    }

    // Set the library path to include the executable path but none of the
    // extensions
    options[i+3].optionString = (char *)aLibPath.getStr();
    options[i+3].extraInfo = NULL;

    // Set miscellaneous optimizations for the JVM
    options[i+4].optionString = "-Xrs";
    options[i+4].extraInfo = NULL;

    // We need to turn off some of Java 1.4's graphics optimizations as
    // they cause full screen window positioning, clipping, and image
    // drawing speed to get messed up
    options[i+5].optionString = "-Dapple.awt.window.position.forceSafeProgrammaticPositioning=false";
    options[i+5].extraInfo = NULL;

    // Fix bug 1800 by explicitly setting the look and feel to Aqua
    options[i+6].optionString = "-Dswing.defaultlaf=apple.laf.AquaLookAndFeel";
    options[i+6].extraInfo = NULL;

    // Java 1.5 and higher on Leopard needs Quartz to be explicitly turned on
    options[i+7].optionString = "-Dapple.awt.graphics.UseQuartz=true";
    options[i+7].extraInfo = NULL;

    // Set the Java max memory to the greater of half of physical user
    // memory or 256 MB.
    int pMib[2];
    size_t nMinMem = 256 * 1024 * 1024;
    size_t nMaxMem = nMinMem * 4;
    size_t nUserMem = 0;
    size_t nLen = sizeof( nUserMem );
    pMib[0] = CTL_HW;
    pMib[1] = HW_USERMEM;
    if ( !sysctl( pMib, 2, &nUserMem, &nLen, NULL, 0 ) )
        nUserMem /= 2;
    if ( nUserMem > nMaxMem )
        nUserMem = nMaxMem;
    else if ( nUserMem < nMinMem )
        nUserMem = nMinMem;
    rtl::OStringBuffer aBuf( "-Xmx" );
    aBuf.append( (sal_Int32)( nUserMem / ( 1024 * 1024 ) ) );
    aBuf.append( "m" );
    options[i+8].optionString = (char *)aBuf.makeStringAndClear().getStr();
    options[i+8].extraInfo = NULL;
   
    vm_args.version= JNI_VERSION_1_4;
#else	// USE_JAVA
    vm_args.version= JNI_VERSION_1_2;
#endif	// USE_JAVA
    vm_args.options= options;
#ifdef USE_JAVA
    vm_args.nOptions= cOptions + 9;
#else	// USE_JAVA
    vm_args.nOptions= cOptions + 1;
#endif	// USE_JAVA
    vm_args.ignoreUnrecognized= JNI_TRUE;

    /* We set a global flag which is used by the abort handler in order to
       determine whether it is  should use longjmp to get back into this function.
       That is, the abort handler determines if it is on the same stack as this function
       and then jumps back into this function.
    */
    g_bInGetJavaVM = 1;
    jint err;
    JavaVM * pJavaVM = 0;
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

#ifdef USE_JAVA
        // We cannot trust that the OOo build has honored the -source flag so
        // don't use it here. This will will prevent loading of JVM's that
        //  cannot load all classes built by OOo.
        if (err == 0)
        {
            jclass aClass = (*ppEnv)->FindClass( "JREProperties" );
            if ((*ppEnv)->ExceptionCheck())
            {
                (*ppEnv)->ExceptionDescribe();
                err = 1;
            }
        }

        // The JVM's native drawing methods print many spurious error messages
        // on Mac OS X 10.4 which will flood the system log so we need to
        // filter out the messages
        int fd[2];
		const char *pGrepCmd = "/usr/bin/grep";
        if (err == 0 && !access( pGrepCmd, R_OK | X_OK ) && !pipe(fd))
        {
            pid_t pid = fork();
            if (!pid)
            {
                // Child process executes grep -v
                dup2(1, 2);
                dup2(fd[0], 0);
                close(fd[0]);
                close(fd[1]);
                execlp( pGrepCmd, pGrepCmd, "-v", "^ERROR: ", NULL );
                exit(0);
            }
            else if (pid > 0)
            {
                // Parent process redirects stderr to pipe
                dup2(fd[1], 2);
                close(fd[0]);
                close(fd[1]);
            }
            else
            {
                close(fd[0]);
                close(fd[1]);
            }
        }
#endif	// USE_JAVA
    }
    else
        // set err to a positive number, so as or recognize that an abort (longjmp)
        //occurred
        err= 1;
    
    if(err != 0)
    {
        rtl::OUString message;
        if( err < 0)
        {
            fprintf(stderr,"[Java framework] sunjavaplugin"SAL_DLLEXTENSION
                    "Can not create Java Virtual Machine\n");
            errcode = JFW_PLUGIN_E_VM_CREATION_FAILED;
        }
        else if( err > 0)
        {
            fprintf(stderr,"[Java framework] sunjavaplugin"SAL_DLLEXTENSION
                    "Can not create JavaVirtualMachine, abort handler was called.\n");
            errcode = JFW_PLUGIN_E_VM_CREATION_FAILED;
        }
    }
    else
    {
        *ppVm = pJavaVM;
        JFW_TRACE2("[Java framework] sunjavaplugin"SAL_DLLEXTENSION " has created a VM.\n");
    }
        
        
   return errcode;
}



