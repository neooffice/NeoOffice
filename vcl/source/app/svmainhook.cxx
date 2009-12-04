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
 *    Modified December 2009 by Patrick Luby. NeoOffice is distributed under
 *    GPL only under modification term 3 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#ifndef _TOOLS_H
#include <tools/tools.h>
#endif

#ifndef MACOSX

BOOL ImplSVMainHook( BOOL * )
{
    return FALSE;   // indicate that ImplSVMainHook is not implemented
}

#else
#include <osl/thread.h>
#include <premac.h>
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>
#include <unistd.h>

#ifdef USE_JAVA

#include <rtl/alloc.h>
#include <pthread.h>

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

#define MIN_SVMAIN_STACK_SIZE ( 2 * 1024 * 1024 )

static pthread_attr_t *pSVMainAttr = NULL;
static bool bInCreateSVMainThread = false;

#endif	// USE_JAVA

extern BOOL ImplSVMain();

// ============================================================================

#ifdef USE_JAVA

extern "C" SAL_DLLPUBLIC_EXPORT const pthread_attr_t *NewSVMainThreadAttributes()
{
    // Fix bug 3573 by setting a higher minimum stack size for the thread
    // that SVMain runs on
    if ( bInCreateSVMainThread && GetCurrentEventLoop() == GetMainEventLoop() )
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

static void SourceContextCallBack( void *pInfo )
{
}

struct ThreadContext
{
    BOOL* pRet;
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

    // Force exit since some JVMs won't shutdown when only exit() is invoked
    _exit( 0 );
}

BOOL ImplSVMainHook( BOOL *pbInit )
{
    // Mac OS X requires that any Cocoa code have a CFRunLoop started in the
    // primordial thread. Since all of the AWT classes in Java 1.4 and higher
    // are written in Cocoa, we need to start the CFRunLoop here and run
    // ImplSVMain() in a secondary thread.
    // See http://developer.apple.com/samplecode/simpleJavaLauncher/listing3.html
    // for further details and an example

    CFRunLoopRef runLoopRef = CFRunLoopGetCurrent();
    ThreadContext tcx;
    tcx.pRet = pbInit;  // the return value
    tcx.pRunLoopRef = &runLoopRef;
    oslThread hThreadID = osl_createThread(RunSVMain, &tcx);

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

    osl_joinWithThread( hThreadID );
    osl_destroyThread( hThreadID );

    return TRUE;    // indicate that ImplSVMainHook is implemented
}

#endif // MACOSX
