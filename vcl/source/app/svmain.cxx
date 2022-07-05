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
 *   Modified December 2016 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <sal/config.h>

#include <cassert>

#include "rtl/logfile.hxx"

#include <osl/file.hxx>
#include <osl/signal.h>

#include "tools/debug.hxx"
#include "tools/resmgr.hxx"

#include "comphelper/processfactory.hxx"

#include "unotools/syslocaleoptions.hxx"
#include "vcl/svapp.hxx"
#include "vcl/wrkwin.hxx"
#include "vcl/cvtgrf.hxx"
#include "vcl/image.hxx"
#include "vcl/settings.hxx"
#include "vcl/unowrap.hxx"
#include "vcl/configsettings.hxx"
#include "vcl/lazydelete.hxx"
#include "vcl/embeddedfontshelper.hxx"
#include "vcl/debugevent.hxx"

#ifdef WNT
#include <svsys.h>
#include <process.h>
#include <ole2.h>
#endif

#ifdef ANDROID
#include <cppuhelper/bootstrap.hxx>
#include <jni.h>
#endif

#include "salinst.hxx"
#include "salwtype.hxx"
#include "svdata.hxx"
#include "vcl/svmain.hxx"
#include "dbggui.hxx"
#include "accmgr.hxx"
#include "idlemgr.hxx"
#include "outdev.h"
#include "outfont.hxx"
#include "PhysicalFontCollection.hxx"
#include "print.h"
#include "salgdi.hxx"
#include "salsys.hxx"
#include "saltimer.hxx"
#include "salimestatus.hxx"
#include "impimagetree.hxx"
#include "xconnection.hxx"

#include "vcl/opengl/OpenGLContext.hxx"

#include "osl/process.h"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/frame/Desktop.hpp"

#include "cppuhelper/implbase1.hxx"
#include "uno/current_context.hxx"

#if OSL_DEBUG_LEVEL > 0
#include <typeinfo>
#include "rtl/strbuf.hxx"
#endif

#if defined USE_JAVA && defined MACOSX

#include <unotools/bootstrap.hxx>

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>
 
#include "java/saldata.hxx"

static bool bInImplSVMain = false;

#endif	// USE_JAVA && MACOSX

using namespace ::com::sun::star;

#if defined USE_JAVA && defined MACOSX

static void ImplLoadNativeFont( OUString aPath )
{
    if ( !aPath.getLength() )
        return;

    oslDirectory aDir = NULL;
    if ( osl_openDirectory( aPath.pData, &aDir ) == osl_File_E_None )
    {
        oslDirectoryItem aDirItem = NULL;
        while ( osl_getNextDirectoryItem( aDir, &aDirItem, 16 ) == osl_File_E_None )
        {
            oslFileStatus aStatus;
            memset( &aStatus, 0, sizeof( oslFileStatus ) );
            if ( osl_getFileStatus( aDirItem, &aStatus, osl_FileStatus_Mask_FileURL ) == osl_File_E_None )
                ImplLoadNativeFont( OUString( aStatus.ustrFileURL ) );
        }
        if ( aDirItem )
            osl_releaseDirectoryItem( aDirItem );
    }
    else
    {
        OUString aSysPath;
        if ( osl_getSystemPathFromFileURL( aPath.pData, &aSysPath.pData ) == osl_File_E_None )
        {
            CFStringRef aString = CFStringCreateWithCharacters( NULL, aSysPath.getStr(), aSysPath.getLength() );
            if ( aString )
            {
                CFURLRef aURL = CFURLCreateWithFileSystemPath( NULL, aString, kCFURLPOSIXPathStyle, false );
                if ( aURL )
                {
                    CTFontManagerRegisterFontsForURL( aURL, kCTFontManagerScopeUser, NULL );

                    // Loading our private fonts is a bit flaky so try loading
                    // using the process context just to be safe
                    CTFontManagerRegisterFontsForURL( aURL, kCTFontManagerScopeProcess, NULL );

                    CFRelease( aURL );
                }

                CFRelease( aString );
            }
        }
    }
}

#endif	// USE_JAVA && MACOSX

oslSignalAction SAL_CALL VCLExceptionSignal_impl( void* /*pData*/, oslSignalInfo* pInfo)
{
    static bool bIn = false;

    // if we crash again, bail out immediately
    if ( !bIn )
    {
        sal_uInt16 nVCLException = 0;

        // UAE
        if ( (pInfo->Signal == osl_Signal_AccessViolation)     ||
             (pInfo->Signal == osl_Signal_IntegerDivideByZero) ||
             (pInfo->Signal == osl_Signal_FloatDivideByZero)   ||
             (pInfo->Signal == osl_Signal_DebugBreak) )
            nVCLException = EXC_SYSTEM;

        // RC
        if ((pInfo->Signal == osl_Signal_User) &&
            (pInfo->UserSignal == OSL_SIGNAL_USER_RESOURCEFAILURE) )
            nVCLException = EXC_RSCNOTLOADED;

        // DISPLAY-Unix
        if ((pInfo->Signal == osl_Signal_User) &&
            (pInfo->UserSignal == OSL_SIGNAL_USER_X11SUBSYSTEMERROR) )
            nVCLException = EXC_DISPLAY;

        // Remote-Client
        if ((pInfo->Signal == osl_Signal_User) &&
            (pInfo->UserSignal == OSL_SIGNAL_USER_RVPCONNECTIONERROR) )
            nVCLException = EXC_REMOTE;

        if ( nVCLException )
        {
            bIn = true;
#if defined USE_JAVA && defined MACOSX
            GetSalData()->mbInSignalHandler = true;
#endif	// USE_JAVA && MACOSX

            SolarMutexGuard aLock;

            // do not stop timer because otherwise the UAE-Box will not be painted as well
            ImplSVData* pSVData = ImplGetSVData();
            if ( pSVData->mpApp )
            {
                sal_uInt16 nOldMode = Application::GetSystemWindowMode();
                Application::SetSystemWindowMode( nOldMode & ~SYSTEMWINDOW_MODE_NOAUTOMODE );
                pSVData->mpApp->Exception( nVCLException );
                Application::SetSystemWindowMode( nOldMode );
            }
#if defined USE_JAVA && defined MACOSX
            GetSalData()->mbInSignalHandler = false;
#endif	// USE_JAVA && MACOSX
            bIn = false;

            return osl_Signal_ActCallNextHdl;
        }
    }

    return osl_Signal_ActCallNextHdl;

}

int ImplSVMain()
{
#if defined USE_JAVA && defined MACOSX
    bool bOldInImplSVMain = bInImplSVMain;
    bInImplSVMain = true;
#endif	// USE_JAVA && MACOSX

    // The 'real' SVMain()
    ImplSVData* pSVData = ImplGetSVData();

    DBG_ASSERT( pSVData->mpApp, "no instance of class Application" );

    int nReturn = EXIT_FAILURE;

    bool bInit = InitVCL();

    if( bInit )
    {
        // call application main
        pSVData->maAppData.mbInAppMain = true;
        nReturn = pSVData->mpApp->Main();
        pSVData->maAppData.mbInAppMain = false;
    }

    if( pSVData->mxDisplayConnection.is() )
    {
        pSVData->mxDisplayConnection->terminate();
        pSVData->mxDisplayConnection.clear();
    }

    // This is a hack to work around the problem of the asynchronous nature
    // of bridging accessibility through Java: on shutdown there might still
    // be some events in the AWT EventQueue, which need the SolarMutex which
    // - on the other hand - is destroyed in DeInitVCL(). So empty the queue
    // here ..
    if( pSVData->mxAccessBridge.is() )
    {
      sal_uLong nCount = Application::ReleaseSolarMutex();
      pSVData->mxAccessBridge->dispose();
      Application::AcquireSolarMutex(nCount);
      pSVData->mxAccessBridge.clear();
    }

    DeInitVCL();
#if defined USE_JAVA && defined MACOSX
    bInImplSVMain = bOldInImplSVMain;
#endif	// USE_JAVA && MACOSX
    return nReturn;
}

int SVMain()
{
#if defined USE_JAVA && defined MACOSX
    // Attempt to fix haxie bugs that cause bug 2912 by calling
    // CFRunLoopGetMain() in the main thread before we have created a
    // secondary thread
    CFRunLoopRef aMainRunLoop = CFRunLoopGetMain();
    if ( CFRunLoopGetCurrent() == aMainRunLoop )
    {
        // Activate the fonts in the "user/fonts" directory. Fix bug 2733 on
        // Leopard by loading the fonts before Java is ever loaded.
        OUString aUserPath;
        if ( utl::Bootstrap::locateUserInstallation( aUserPath ) == utl::Bootstrap::PATH_EXISTS )
        {
            if ( aUserPath.getLength() )
            {
                aUserPath += OUString::createFromAscii( "/user/fonts" );
                ImplLoadNativeFont( aUserPath );
            }
        }

        // Activate the fonts in the "Resources/fonts/truetype" directory
        OUString aBasePath;
        if ( utl::Bootstrap::locateBaseInstallation( aBasePath ) == utl::Bootstrap::PATH_EXISTS )
        {
            if ( aBasePath.getLength() )
            {
                aBasePath += OUString::createFromAscii( "/Resources/fonts/truetype" );
                ImplLoadNativeFont( aBasePath );
            }
        }
    }
#endif	// USE_JAVA && MACOSX

    int nRet;
    if( !Application::IsConsoleOnly() && ImplSVMainHook( &nRet ) )
        return nRet;
    else
        return ImplSVMain();
}

// This variable is set when no Application object has been instantiated
// before InitVCL is called
static Application *        pOwnSvApp = NULL;

// Exception handler. pExceptionHandler != NULL => VCL already inited
oslSignalHandler   pExceptionHandler = NULL;

class DummyApplication : public Application
{
public:
    int                Main() SAL_OVERRIDE { return EXIT_SUCCESS; };
};

class DesktopEnvironmentContext: public cppu::WeakImplHelper1< com::sun::star::uno::XCurrentContext >
{
public:
    DesktopEnvironmentContext( const com::sun::star::uno::Reference< com::sun::star::uno::XCurrentContext > & ctx)
        : m_xNextContext( ctx ) {}

    // XCurrentContext
    virtual com::sun::star::uno::Any SAL_CALL getValueByName( const OUString& Name )
            throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    com::sun::star::uno::Reference< com::sun::star::uno::XCurrentContext > m_xNextContext;
};

uno::Any SAL_CALL DesktopEnvironmentContext::getValueByName( const OUString& Name) throw (uno::RuntimeException, std::exception)
{
    uno::Any retVal;

    if ( Name == "system.desktop-environment" )
    {
        retVal = uno::makeAny( Application::GetDesktopEnvironment() );
    }
    else if( m_xNextContext.is() )
    {
        // Call next context in chain if found
        retVal = m_xNextContext->getValueByName( Name );
    }
    return retVal;
}

bool InitVCL()
{
    if( pExceptionHandler != NULL )
        return false;

    EmbeddedFontsHelper::clearTemporaryFontFiles();

    if( ! ImplGetSVData() )
        ImplInitSVData();

    if( !ImplGetSVData()->mpApp )
    {
        pOwnSvApp = new DummyApplication();
    }
    InitSalMain();

    ImplSVData* pSVData = ImplGetSVData();

    // remember Main-Thread-Id
    pSVData->mnMainThreadId = ::osl::Thread::getCurrentIdentifier();

    // Initialize Sal
#if defined USE_JAVA && defined MACOSX
    pSVData->mpDefInst = CreateSalInstance( bInImplSVMain );
#else	// USE_JAVA && MACOSX
    pSVData->mpDefInst = CreateSalInstance();
#endif	// USE_JAVA && MACOSX
    if ( !pSVData->mpDefInst )
        return false;

    // Desktop Environment context (to be able to get value of "system.desktop-environment" as soon as possible)
    com::sun::star::uno::setCurrentContext(
        new DesktopEnvironmentContext( com::sun::star::uno::getCurrentContext() ) );

    // Initialize application instance (should be done after initialization of VCL SAL part)
    if( pSVData->mpApp )
        // call init to initialize application class
        // soffice/sfx implementation creates the global service manager
        pSVData->mpApp->Init();

    pSVData->mpDefInst->AfterAppInit();

    // Fetch AppFileName and make it absolute before the workdir changes...
    OUString aExeFileName;
    osl_getExecutableFile( &aExeFileName.pData );

    // convert path to native file format
    OUString aNativeFileName;
    osl::FileBase::getSystemPathFromFileURL( aExeFileName, aNativeFileName );
    pSVData->maAppData.mpAppFileName = new OUString( aNativeFileName );

    // Initialize global data
    pSVData->maGDIData.mpScreenFontList     = new PhysicalFontCollection;
    pSVData->maGDIData.mpScreenFontCache    = new ImplFontCache;
    pSVData->maGDIData.mpGrfConverter       = new GraphicConverter;

    // Set exception handler
    pExceptionHandler = osl_addSignalHandler(VCLExceptionSignal_impl, NULL);

    DBGGUI_INIT_SOLARMUTEXCHECK();

#if OSL_DEBUG_LEVEL > 0
    DebugEventInjector::getCreate();
#endif

    return true;
}

#ifdef ANDROID

extern "C" __attribute__ ((visibility("default"))) void
InitVCLWrapper()
{
    uno::Reference<uno::XComponentContext> xContext( cppu::defaultBootstrap_InitialComponentContext() );
    uno::Reference<lang::XMultiComponentFactory> xFactory( xContext->getServiceManager() );

    uno::Reference<lang::XMultiServiceFactory> xSM( xFactory, uno::UNO_QUERY_THROW );

    comphelper::setProcessServiceFactory( xSM );

    InitVCL();
}

#endif

namespace
{

/** Serves for destroying the VCL UNO wrapper as late as possible. This avoids
  crash at exit in some special cases when a11y is enabled (e.g., when
  a bundled extension is registered/deregistered during startup, forcing exit
  while the app is still in splash screen.)
 */
class VCLUnoWrapperDeleter : public cppu::WeakImplHelper1<com::sun::star::lang::XEventListener>
{
    virtual void SAL_CALL disposing(lang::EventObject const& rSource) throw(uno::RuntimeException, std::exception) SAL_OVERRIDE;
};

void
VCLUnoWrapperDeleter::disposing(lang::EventObject const& /* rSource */)
    throw(uno::RuntimeException, std::exception)
{
    ImplSVData* const pSVData = ImplGetSVData();
    if (pSVData && pSVData->mpUnoWrapper)
    {
        pSVData->mpUnoWrapper->Destroy();
        pSVData->mpUnoWrapper = NULL;
    }
}

}

void DeInitVCL()
{
    ImplSVData* pSVData = ImplGetSVData();
    pSVData->mbDeInit = true;

    vcl::DeleteOnDeinitBase::ImplDeleteOnDeInit();

    // give ime status a chance to destroy its own windows
    delete pSVData->mpImeStatus;
    pSVData->mpImeStatus = NULL;

#if OSL_DEBUG_LEVEL > 0
    OStringBuffer aBuf( 256 );
    aBuf.append( "DeInitVCL: some top Windows are still alive\n" );
    long nTopWindowCount = Application::GetTopWindowCount();
    long nBadTopWindows = nTopWindowCount;
    for( long i = 0; i < nTopWindowCount; i++ )
    {
        vcl::Window* pWin = Application::GetTopWindow( i );
        // default window will be destroyed further down
        // but may still be useful during deinit up to that point
        if( pWin == pSVData->mpDefaultWin )
            nBadTopWindows--;
        else
        {
            aBuf.append( "text = \"" );
            aBuf.append( OUStringToOString( pWin->GetText(), osl_getThreadTextEncoding() ) );
            aBuf.append( "\" type = \"" );
            aBuf.append( typeid(*pWin).name() );
            aBuf.append( "\", ptr = 0x" );
            aBuf.append( sal_Int64( pWin ), 16 );
            aBuf.append( "\n" );
        }
    }
    DBG_ASSERT( nBadTopWindows==0, aBuf.getStr() );
#endif

    ImplImageTreeSingletonRef()->shutDown();

    osl_removeSignalHandler( pExceptionHandler);
    pExceptionHandler = NULL;

    // free global data
    delete pSVData->maGDIData.mpGrfConverter;

    if( pSVData->mpSettingsConfigItem )
        delete pSVData->mpSettingsConfigItem, pSVData->mpSettingsConfigItem = NULL;

    if ( pSVData->maAppData.mpIdleMgr )
        delete pSVData->maAppData.mpIdleMgr;
    Timer::ImplDeInitTimer();

    if ( pSVData->maWinData.mpMsgBoxImgList )
    {
        delete pSVData->maWinData.mpMsgBoxImgList;
        pSVData->maWinData.mpMsgBoxImgList = NULL;
    }
    if ( pSVData->maCtrlData.mpCheckImgList )
    {
        delete pSVData->maCtrlData.mpCheckImgList;
        pSVData->maCtrlData.mpCheckImgList = NULL;
    }
    if ( pSVData->maCtrlData.mpRadioImgList )
    {
        delete pSVData->maCtrlData.mpRadioImgList;
        pSVData->maCtrlData.mpRadioImgList = NULL;
    }
    if ( pSVData->maCtrlData.mpPinImgList )
    {
        delete pSVData->maCtrlData.mpPinImgList;
        pSVData->maCtrlData.mpPinImgList = NULL;
    }
    if ( pSVData->maCtrlData.mpSplitHPinImgList )
    {
        delete pSVData->maCtrlData.mpSplitHPinImgList;
        pSVData->maCtrlData.mpSplitHPinImgList = NULL;
    }
    if ( pSVData->maCtrlData.mpSplitVPinImgList )
    {
        delete pSVData->maCtrlData.mpSplitVPinImgList;
        pSVData->maCtrlData.mpSplitVPinImgList = NULL;
    }
    if ( pSVData->maCtrlData.mpSplitHArwImgList )
    {
        delete pSVData->maCtrlData.mpSplitHArwImgList;
        pSVData->maCtrlData.mpSplitHArwImgList = NULL;
    }
    if ( pSVData->maCtrlData.mpSplitVArwImgList )
    {
        delete pSVData->maCtrlData.mpSplitVArwImgList;
        pSVData->maCtrlData.mpSplitVArwImgList = NULL;
    }
    if ( pSVData->maCtrlData.mpDisclosurePlus )
    {
        delete pSVData->maCtrlData.mpDisclosurePlus;
        pSVData->maCtrlData.mpDisclosurePlus = NULL;
    }
    if ( pSVData->maCtrlData.mpDisclosureMinus )
    {
        delete pSVData->maCtrlData.mpDisclosureMinus;
        pSVData->maCtrlData.mpDisclosureMinus = NULL;
    }
    if ( pSVData->mpDefaultWin )
    {
        OpenGLContext* pContext = pSVData->mpDefaultWin->GetGraphics()->GetOpenGLContext();
        if( pContext )
        {
#ifdef DBG_UTIL
            pContext->DeRef(NULL);
#else
            pContext->DeRef();
#endif
        }
        delete pSVData->mpDefaultWin;
        pSVData->mpDefaultWin = NULL;
    }

    DBGGUI_DEINIT_SOLARMUTEXCHECK();

    if ( pSVData->mpUnoWrapper )
    {
        try
        {
            uno::Reference<frame::XDesktop2> const xDesktop = frame::Desktop::create(
                    comphelper::getProcessComponentContext() );
            xDesktop->addEventListener(new VCLUnoWrapperDeleter());
        }
        catch (uno::Exception const&)
        {
            // ignore
        }
    }

    if( pSVData->mpApp || pSVData->maDeInitHook.IsSet() )
    {
        sal_uLong nCount = Application::ReleaseSolarMutex();
        // call deinit to deinitialize application class
        // soffice/sfx implementation disposes the global service manager
        // Warning: After this call you can't call uno services
        if( pSVData->mpApp )
        {
            pSVData->mpApp->DeInit();
        }
        if( pSVData->maDeInitHook.IsSet() )
        {
            pSVData->maDeInitHook.Call(0);
        }
        Application::AcquireSolarMutex(nCount);
    }

    if ( pSVData->maAppData.mpSettings )
    {
        if ( pSVData->maAppData.mpCfgListener )
        {
            pSVData->maAppData.mpSettings->GetSysLocale().GetOptions().RemoveListener( pSVData->maAppData.mpCfgListener );
            delete pSVData->maAppData.mpCfgListener;
        }

        delete pSVData->maAppData.mpSettings;
        pSVData->maAppData.mpSettings = NULL;
    }
    if ( pSVData->maAppData.mpAccelMgr )
    {
        delete pSVData->maAppData.mpAccelMgr;
        pSVData->maAppData.mpAccelMgr = NULL;
    }
    if ( pSVData->maAppData.mpAppFileName )
    {
        delete pSVData->maAppData.mpAppFileName;
        pSVData->maAppData.mpAppFileName = NULL;
    }
    if ( pSVData->maAppData.mpAppName )
    {
        delete pSVData->maAppData.mpAppName;
        pSVData->maAppData.mpAppName = NULL;
    }
    if ( pSVData->maAppData.mpDisplayName )
    {
        delete pSVData->maAppData.mpDisplayName;
        pSVData->maAppData.mpDisplayName = NULL;
    }
    if ( pSVData->maAppData.mpEventListeners )
    {
        delete pSVData->maAppData.mpEventListeners;
        pSVData->maAppData.mpEventListeners = NULL;
    }
    if ( pSVData->maAppData.mpKeyListeners )
    {
        delete pSVData->maAppData.mpKeyListeners;
        pSVData->maAppData.mpKeyListeners = NULL;
    }
    if ( pSVData->maAppData.mpPostYieldListeners )
    {
        delete pSVData->maAppData.mpPostYieldListeners;
        pSVData->maAppData.mpPostYieldListeners = NULL;
    }

    if ( pSVData->maAppData.mpFirstHotKey )
        ImplFreeHotKeyData();
    if ( pSVData->maAppData.mpFirstEventHook )
        ImplFreeEventHookData();

    if (pSVData->mpBlendFrameCache)
        delete pSVData->mpBlendFrameCache, pSVData->mpBlendFrameCache = NULL;

    ImplDeletePrnQueueList();
    delete pSVData->maGDIData.mpScreenFontList;
    pSVData->maGDIData.mpScreenFontList = NULL;
    delete pSVData->maGDIData.mpScreenFontCache;
    pSVData->maGDIData.mpScreenFontCache = NULL;

    if ( pSVData->mpResMgr )
    {
        delete pSVData->mpResMgr;
        pSVData->mpResMgr = NULL;
    }

    ResMgr::DestroyAllResMgr();

    // destroy all Sal interfaces before destorying the instance
    // and thereby unloading the plugin
    delete pSVData->mpSalSystem;
    pSVData->mpSalSystem = NULL;
    delete pSVData->mpSalTimer;
    pSVData->mpSalTimer = NULL;

    // Deinit Sal
#if defined USE_JAVA && defined MACOSX
    // Fix random crashing in native callbacks that get called after destroying
    // the SalInstance by destroying it in ImplDeInitSVData()
#else	// USE_JAVA && MACOSX
    DestroySalInstance( pSVData->mpDefInst );
#endif  // USE_JAVA && MACOSX

    if( pOwnSvApp )
    {
        delete pOwnSvApp;
        pOwnSvApp = NULL;
    }

    EmbeddedFontsHelper::clearTemporaryFontFiles();
}

// only one call is allowed
struct WorkerThreadData
{
    oslWorkerFunction   pWorker;
    void *              pThreadData;
    WorkerThreadData( oslWorkerFunction pWorker_, void * pThreadData_ )
        : pWorker( pWorker_ )
        , pThreadData( pThreadData_ )
    {
    }
};

#ifdef WNT
static HANDLE hThreadID = 0;
static unsigned __stdcall _threadmain( void *pArgs )
{
    OleInitialize( NULL );
    ((WorkerThreadData*)pArgs)->pWorker( ((WorkerThreadData*)pArgs)->pThreadData );
    delete (WorkerThreadData*)pArgs;
    OleUninitialize();
    hThreadID = 0;
    return 0;
}
#else
static oslThread hThreadID = 0;
extern "C"
{
static void SAL_CALL MainWorkerFunction( void* pArgs )
{
    ((WorkerThreadData*)pArgs)->pWorker( ((WorkerThreadData*)pArgs)->pThreadData );
    delete (WorkerThreadData*)pArgs;
    hThreadID = 0;
}
} // extern "C"
#endif

void CreateMainLoopThread( oslWorkerFunction pWorker, void * pThreadData )
{
#ifdef WNT
    // sal thread always call CoInitializeEx, so a sysdepen implementation is necessary

    unsigned uThreadID;
    hThreadID = (HANDLE)_beginthreadex(
        NULL,       // no security handle
        0,          // stacksize 0 means default
        _threadmain,    // thread worker function
        new WorkerThreadData( pWorker, pThreadData ),       // arguments for worker function
        0,          // 0 means: create immediately otherwise use CREATE_SUSPENDED
        &uThreadID );   // thread id to fill
#else
    hThreadID = osl_createThread( MainWorkerFunction, new WorkerThreadData( pWorker, pThreadData ) );
#endif
}

void JoinMainLoopThread()
{
    if( hThreadID )
    {
#ifdef WNT
        WaitForSingleObject(hThreadID, INFINITE);
#else
        osl_joinWithThread(hThreadID);
        osl_destroyThread( hThreadID );
#endif
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
