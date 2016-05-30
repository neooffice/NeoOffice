/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 * This file incorporates work covered by the following license notice:
 * 
 *   Modified May 2016 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 4
 *   of the Apache License, Version 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"
#include <tools/tools.h>

#ifndef MACOSX

sal_Bool ImplSVMainHook( sal_Bool * )
{
    return sal_False;   // indicate that ImplSVMainHook is not implemented
}

#else
// MACOSX cocoa implementation of ImplSVMainHook is in aqua/source/app/salinst.cxx
#if !defined QUARTZ || defined USE_JAVA // MACOSX (X11) needs the CFRunLoop()
#include <osl/thread.h>
#include <premac.h>
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>
#include <unistd.h>

#ifdef USE_JAVA

#include <rtl/alloc.h>
#include <pthread.h>

#include <premac.h>
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>

#include "svmainhook_cocoa.h"

#define MIN_SVMAIN_STACK_SIZE ( 2 * 1024 * 1024 )

static pthread_attr_t *pSVMainAttr = NULL;
static bool bInCreateSVMainThread = false;

#endif	// USE_JAVA

extern sal_Bool ImplSVMain();

// ============================================================================


#ifdef USE_JAVA

extern "C" SAL_DLLPUBLIC_EXPORT const pthread_attr_t *NewSVMainThreadAttributes()
{
    // Fix bug 3573 by setting a higher minimum stack size for the thread
    // that SVMain runs on
    if ( bInCreateSVMainThread && CFRunLoopGetCurrent() == CFRunLoopGetMain() )
    {
        if ( !pSVMainAttr )
        {
            pSVMainAttr = (pthread_attr_t *)rtl_allocateMemory( sizeof( pthread_attr_t ) );
            if ( pSVMainAttr )
            {
                if ( !pthread_attr_init( pSVMainAttr ) )
                {
                    size_t nStacksize;
                    if ( !pthread_attr_getstacksize( pSVMainAttr, &nStacksize ) && nStacksize < MIN_SVMAIN_STACK_SIZE )
                        pthread_attr_setstacksize( pSVMainAttr, MIN_SVMAIN_STACK_SIZE );
                }
                else
                {
                    rtl_freeMemory( pSVMainAttr );
                    pSVMainAttr = NULL;
                }
            }
        }
    }

    return pSVMainAttr;
}

#endif	 // USE_JAVA

#ifndef USE_JAVA

static void SourceContextCallBack( void *pInfo )
{
}

#endif	// USE_JAVA

struct ThreadContext
{
    sal_Bool* pRet;
    CFRunLoopRef* pRunLoopRef;
};

static void RunSVMain(void *pData)
{
    ThreadContext* tcx = reinterpret_cast<ThreadContext*>(pData);

    // busy waiting (ok in this case) until the run loop is
    // running
    while (!CFRunLoopIsWaiting(*tcx->pRunLoopRef))
        usleep(100);
    	
    *tcx->pRet = ImplSVMain();

#ifdef USE_JAVA
    NSApplication_terminate();
#endif	// USE_JAVA

    // Force exit since some JVMs won't shutdown when only exit() is invoked
    _exit( 0 );
}

sal_Bool ImplSVMainHook( sal_Bool *pbInit )
{
    // Mac OS X requires that any Cocoa code have a CFRunLoop started in the
    // primordial thread. Since all of the AWT classes in Java 1.4 and higher
    // are written in Cocoa, we need to start the CFRunLoop here and run
    // ImplSVMain() in a secondary thread.
    // See http://developer.apple.com/samplecode/simpleJavaLauncher/listing3.html
    // for further details and an example

    CFRunLoopRef runLoopRef = CFRunLoopGetCurrent();
#ifdef USE_JAVA
    if ( runLoopRef != CFRunLoopGetMain() )
        return sal_False;
#endif	// USE_JAVA
    ThreadContext tcx;
    tcx.pRet = pbInit;  // the return value
    tcx.pRunLoopRef = &runLoopRef;
#ifdef USE_JAVA
    bInCreateSVMainThread = true;
#endif	// USE_JAVA
    oslThread hThreadID = osl_createThread(RunSVMain, &tcx);
#ifdef USE_JAVA
    bInCreateSVMainThread = false;
#endif	// USE_JAVA

#ifdef USE_JAVA
    NSApplication_run();
#else	// USE_JAVA
    // Start the CFRunLoop
    CFRunLoopSourceContext aSourceContext;
    aSourceContext.version = 0;
    aSourceContext.info = NULL;
    aSourceContext.retain = NULL;
    aSourceContext.release = NULL;
    aSourceContext.copyDescription = NULL;
    aSourceContext.equal = NULL;
    aSourceContext.hash = NULL;
    aSourceContext.schedule = NULL;
    aSourceContext.cancel = NULL;
    aSourceContext.perform = &SourceContextCallBack;
    CFRunLoopSourceRef aSourceRef = CFRunLoopSourceCreate(NULL, 0, &aSourceContext);
    CFRunLoopAddSource(runLoopRef, aSourceRef, kCFRunLoopCommonModes);
    CFRunLoopRun();
#endif	// USE_JAVA

    osl_joinWithThread( hThreadID );
    osl_destroyThread( hThreadID );

    return sal_True;    // indicate that ImplSVMainHook is implemented
}

#endif // !QUARTZ || USE_JAVA
#endif
