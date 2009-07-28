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
 *    Modified July 2009 by Patrick Luby. NeoOffice is distributed under
 *    GPL only under modification term 3 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"

#ifndef _CONNECTIVITY_MAB_NS_INCLUDE_HXX_
#include <MNSInclude.hxx>
#endif

#include "mozilla_nsinit.h"

#include <sal/types.h>
#include <osl/diagnose.h>
#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif

#ifndef _CONNECTIVITY_MAB_NS_INIT_HXX_
#include <MNSInit.hxx>
#endif

#ifndef CONNECTIVITY_MOZAB_MCONFIGACCESS_HXX
#include "MConfigAccess.hxx"
#endif

#ifndef _CONNECTIVITY_MAB_NS_DECLARES_HXX_
#include "MNSDeclares.hxx"
#endif

#ifndef _THREAD_HXX_
#include <osl/thread.hxx>
#endif

#ifndef _MNSTERMINATELISTENER_HXX
#include <MNSTerminateListener.hxx>
#endif

static nsIServiceManager*	sServiceManager = nsnull;
static sal_Int32 			sInitCounter = 0;
static sal_Bool 			s_bProfilePresentAfterInitialized = sal_False;

static NS_DEFINE_CID(kProfileCID, NS_PROFILE_CID);
static NS_DEFINE_CID(kPrefCID, NS_PREF_CID);

static NS_DEFINE_CID(kEventQueueServiceCID, NS_EVENTQUEUESERVICE_CID);
static oslThread m_Mozilla_UI_Thread;
static PRBool aLive=1;
static ::osl::Condition    m_aUI_Thread_Condition;

#define HACK_AROUND_THREADING_ISSUES
#define HACK_AROUND_NONREENTRANT_INITXPCOM

#ifdef HACK_AROUND_NONREENTRANT_INITXPCOM
// XXX hack class to clean up XPCOM when this module is unloaded
static PRBool sXPCOMInitializedFlag = PR_FALSE;
#endif


extern "C" void NS_SetupRegistry();

 const PRUnichar* determineProfile( PRUnichar const* const* _pValidProfiles, const PRUint32 _nValidProfiles )
{
	// the fallback for the to-be-used user profile: the first profile
	const PRUnichar* pUsedProfile = *_pValidProfiles;

	// have a look what the configuration suggests as preferred profile
	const PRUnichar* pPreferredProfile = static_cast< const PRUnichar* >( getUserProfile( ) );
	if ( pPreferredProfile && *pPreferredProfile )
	{
		PRUnichar const* const* pLoop = _pValidProfiles;
		PRUnichar const* const* pLoopEnd = pLoop + _nValidProfiles;
		for ( ; pLoop != pLoopEnd; ++pLoop )
		{
			// compare the current and the preferred profile
			// (by looping through the characters)
			const PRUnichar* pCurrent	= *pLoop;
			const PRUnichar* pPref		= pPreferredProfile;
			while ( *pCurrent && ( *pCurrent == *pPref ) )	// testing one of them against 0 is enough because of the second clause
			{
				++pCurrent;
				++pPref;
			}
			if ( *pCurrent == *pPref )
				// the are equal
				break;
		}

		if ( pLoop != pLoopEnd )
			pUsedProfile = *pLoop;
		return pUsedProfile;
	}
	else
		return NULL;
}


sal_Bool MNS_InitXPCOM(sal_Bool* aProfileExists)
{
	nsresult rv;
    OSL_TRACE( "IN : MNS_InitXPCOM() \n" );
	// Reentrant calls to this method do nothing except increment a counter

#ifdef HACK_AROUND_NONREENTRANT_INITXPCOM
	// The first time, add another increment so that it'll be left until exit
	// for the final cleanup to happen
	sInitCounter++;
#endif // HACK_AROUND_NONREENTRANT_INITXPCOM

	// Initialise XPCOM
#ifdef HACK_AROUND_NONREENTRANT_INITXPCOM
	// Can't call NS_InitXPCom more than once or things go boom!
	if (!sXPCOMInitializedFlag)
#endif
	{
        nsCOMPtr<nsILocalFile> binDir;
        // Note: if getenv() returns NULL, mozilla will default to using MOZILLA_FIVE_HOME in the NS_InitXPCOM2()
        // The NS_NewNativeLocalFile() will accept NULL as its first parameter.
        char * env = getenv("OPENOFFICE_MOZILLA_FIVE_HOME");
        if (env)
        {
            nsDependentCString sPath(env);
            rv = NS_NewNativeLocalFile(sPath, PR_TRUE, getter_AddRefs(binDir));
            if (NS_FAILED(rv))
                return sal_False;
        }



	// Initialise XPCOM
    NS_InitXPCOM2(&sServiceManager, binDir, NULL);

//		if (!sServiceManager)
//			return sal_False;

#ifdef HACK_AROUND_NONREENTRANT_INITXPCOM
		sXPCOMInitializedFlag = PR_TRUE;
#endif
	}


    // Create the Event Queue for the UI thread...
    //
    // If an event queue already exists for the thread, then
    // CreateThreadEventQueue(...) will fail...
    // CreateThread0ueue(...) will fail...
    nsCOMPtr<nsIEventQueueService> eventQService(
                do_GetService(NS_EVENTQUEUESERVICE_CONTRACTID, &rv));
    if (NS_FAILED(rv))
      return NS_SUCCEEDED( rv );

    eventQService->CreateThreadEventQueue();

//	nsCOMPtr<nsIObserver> mStartupNotifier = do_CreateInstance(NS_APPSTARTUPNOTIFIER_CONTRACTID, &rv);
//	if(NS_FAILED(rv))
//		return rv;
//	mStartupNotifier->Observe(nsnull, APPSTARTUP_TOPIC, nsnull);

#ifdef HACK_AROUND_THREADING_ISSUES
    // XXX force certain objects to be created on the main thread
    nsCOMPtr<nsIStringBundleService> sBundleService;
    sBundleService = do_GetService(NS_STRINGBUNDLE_CONTRACTID, &rv);
    if (NS_SUCCEEDED(rv))
    {
        nsCOMPtr<nsIStringBundle> stringBundle;
        const char* propertyURL = "chrome://necko/locale/necko.properties";
        rv = sBundleService->CreateBundle(propertyURL,
                                          getter_AddRefs(stringBundle));
    }
#endif

	// Need to create a Pref Service
	nsCOMPtr< nsIPref > thePref = do_GetService( kPrefCID, &rv );
	if (NS_SUCCEEDED(rv) )
	{
		OSL_TRACE("Created an nsIPref i/f\n");
		thePref->ReadUserPrefs( nsnull );
		*aProfileExists = sal_True ;
		s_bProfilePresentAfterInitialized = sal_True;
	}
	OSL_TRACE( "OUT : MNS_InitXPCOM() - XPCOM Init\n" );

	return sal_True;
}

void MNS_XPCOM_EventLoop()
{
    OSL_TRACE( "IN : MNS_XPCOM_EventLoop() \n" );
	nsresult rv;
	nsCOMPtr<nsIEventQueue> eventQ;
	nsCOMPtr<nsIEventQueueService>  eventQService;
	rv=nsServiceManager::GetService(kEventQueueServiceCID,
                                 NS_GET_IID(nsIEventQueueService),
                                 getter_AddRefs(eventQService));

	if (NS_SUCCEEDED(rv))
	{
	  rv = eventQService->GetThreadEventQueue(NS_CURRENT_THREAD , getter_AddRefs(eventQ));
	  if (NS_FAILED(rv))
	  {
		  rv = eventQService->CreateThreadEventQueue();
		  if (NS_FAILED(rv))
			return ;
		  else
			rv = eventQService->GetThreadEventQueue(NS_CURRENT_THREAD, getter_AddRefs(eventQ));
	  }
	}

	if (NS_FAILED(rv)) return ;

	PLEvent* event = nsnull;
    m_aUI_Thread_Condition.set(); //we are ready to recive event
    do
    {
        rv = eventQ->GetEvent(&event);
		if (NS_FAILED(rv))
			return ;
        if (event)
	    eventQ->HandleEvent(event);
#ifdef USE_JAVA
    // Fix excessive CPU reported in the following forum topic by lengthening
    // the timeout time if no event was found:
    // http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=55855#55855
    }while ( PR_SUCCESS == PR_Sleep( PR_MillisecondsToInterval(event ? 1 : 100)) && aLive );
#else	// USE_JAVA
    }while ( PR_SUCCESS == PR_Sleep( PR_MillisecondsToInterval(1)) && aLive );
#endif	// USE_JAVA

    eventQ->ProcessPendingEvents();
    OSL_TRACE( "OUT : MNS_XPCOM_EventLoop() \n" );
}

extern "C" void MNS_Mozilla_UI_Thread( void *arg )
{
	aLive=1;
    OSL_TRACE( "IN : MNS_Mozilla_UI_Thread() \n" );
	UI_Thread_ARGS * args = (UI_Thread_ARGS*) arg;
    sal_Bool* aProfileExists=args->bProfileExists;
	delete args;
	args=NULL;

	//Init xpcom
	if (!MNS_InitXPCOM(aProfileExists))
	{
        m_aUI_Thread_Condition.set(); // error happened
		return;
	}

	//do the mozilla event loop
	MNS_XPCOM_EventLoop();
	//we are interupted

	if (sServiceManager)
	{
		NS_RELEASE(sServiceManager);

	// Terminate XPCOM & cleanup
#ifndef HACK_AROUND_NONREENTRANT_INITXPCOM
		NS_ShutdownXPCOM(sServiceManager);
#endif
	}

    m_aUI_Thread_Condition.set();	//release all blocks

    OSL_TRACE( "OUT : MNS_Mozilla_UI_Thread() \n" );

}


sal_Bool MNS_Init(sal_Bool& aProfileExists)
{
	aProfileExists = sal_False ;

	OSL_TRACE( "IN : MNS_Init() \n" );
	// Reentrant calls to this method do nothing except increment a counter
	sInitCounter++;
	if (sInitCounter > 1) {
		OSL_TRACE( "IN : MNS_Init() wait for xpcom to be initted \n" );
        //wait for xpcom to be initted
		m_aUI_Thread_Condition.wait();

        OSL_TRACE( "OUT : MNS_Init() : counter = %d\n", sInitCounter );
		aProfileExists = s_bProfilePresentAfterInitialized;
		return sal_True;
	}

	UI_Thread_ARGS * args = new UI_Thread_ARGS;
    args->bProfileExists = &aProfileExists;

	m_aUI_Thread_Condition.reset();
	m_Mozilla_UI_Thread=osl_createThread(MNS_Mozilla_UI_Thread,
									(void*)args);
	if (!m_Mozilla_UI_Thread)
	{
		return sal_False;
	}

	//wait for xpcom to be initted
	m_aUI_Thread_Condition.wait();

	//Add Terminate Listener to XDesktop to get application exit event
	MNSTerminateListener::addTerminateListener();

	OSL_TRACE( "OUT : MNS_Init() - First Init\n" );

	return sal_True;
}

sal_Bool MNS_Term(sal_Bool aForce)
{
	// Reentrant calls to this method do nothing except decrement a counter
	OSL_TRACE( "IN : MNS_Term() \n" );
	if (!aForce && sInitCounter > 1)
	{
		--sInitCounter;
        OSL_TRACE( "OUT : MNS_Term() : counter = %d\n", sInitCounter );
		return sal_True;
	}
	sInitCounter = 0;

	aLive=0;

	//wait for xpcom to be finished
	TimeValue               timeValue = { 1, 0 };
	m_aUI_Thread_Condition.wait(&timeValue);


	OSL_TRACE( "OUT : MNS_Term() - Final Term\n" );
	return sal_True;
}

