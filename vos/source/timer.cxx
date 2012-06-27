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
 * Modified June 2012 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

#include <osl/time.h>

#include <vos/timer.hxx>
#include <vos/diagnose.hxx>
#include <vos/ref.hxx>
#include <vos/thread.hxx>
#include <vos/conditn.hxx>

#ifdef USE_JAVA

#include <dlfcn.h>
#include <vos/mutex.hxx>

typedef ::vos::IMutex *Application_GetSolarMutexFunc();

#endif	// USE_JAVA

/////////////////////////////////////////////////////////////////////////////
//
// Timer manager
//

class OTimerManagerCleanup;

class NAMESPACE_VOS(OTimerManager) : public NAMESPACE_VOS(OThread)
{
    
public:

    ///
  	OTimerManager();
    
	///
  	~OTimerManager();
    
  	/// register timer
    sal_Bool SAL_CALL registerTimer(NAMESPACE_VOS(OTimer)* pTimer);

  	/// unregister timer
    sal_Bool SAL_CALL unregisterTimer(NAMESPACE_VOS(OTimer)* pTimer);

  	/// lookup timer
    sal_Bool SAL_CALL lookupTimer(const NAMESPACE_VOS(OTimer)* pTimer);

	/// retrieves the "Singleton" TimerManager Instance
	static OTimerManager* SAL_CALL getTimerManager();

    
protected:
    
 	/// worker-function of thread
  	virtual void SAL_CALL run();

    // Checking and triggering of a timer event
    void SAL_CALL checkForTimeout();

    // cleanup Method
    virtual void SAL_CALL onTerminated();
    
  	// sorted-queue data
  	NAMESPACE_VOS(OTimer)*		m_pHead;
    // List Protection
    NAMESPACE_VOS(OMutex)		m_Lock;
    // Signal the insertion of a timer
    NAMESPACE_VOS(OCondition)	m_notEmpty;

    // Synchronize access to OTimerManager
	static NAMESPACE_VOS(OMutex) m_Access;

    // "Singleton Pattern"
    static NAMESPACE_VOS(OTimerManager)* m_pManager;

    friend class OTimerManagerCleanup;
    
};

using namespace vos;

/////////////////////////////////////////////////////////////////////////////
//
// Timer class
//

VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OTimer, vos),
                        VOS_NAMESPACE(OTimer, vos),
                        VOS_NAMESPACE(OObject, vos), 0);

OTimer::OTimer()
{
	m_TimeOut	  = 0;
	m_Expired     = 0;
	m_RepeatDelta = 0;
  	m_pNext       = 0;
}

OTimer::OTimer(const TTimeValue& Time)
{
	m_TimeOut	  = Time;
	m_RepeatDelta = 0;
	m_Expired     = 0;
  	m_pNext       = 0;

	m_TimeOut.normalize();
}

OTimer::OTimer(const TTimeValue& Time, const TTimeValue& Repeat)
{
	m_TimeOut	  = Time;
	m_RepeatDelta = Repeat;
	m_Expired     = 0;
  	m_pNext       = 0;

	m_TimeOut.normalize();
	m_RepeatDelta.normalize();
}

OTimer::~OTimer()
{
    stop();
}

void OTimer::start()
{
	if (! isTicking())
	{
		if (! m_TimeOut.isEmpty())
			setRemainingTime(m_TimeOut);

		OTimerManager *pManager = OTimerManager::getTimerManager();

		VOS_ASSERT(pManager);

		if ( pManager != 0 )
		{
			pManager->registerTimer(this);
		}
	}
}

void OTimer::stop()
{
	OTimerManager *pManager = OTimerManager::getTimerManager();

	VOS_ASSERT(pManager);

	if ( pManager != 0 )
	{
		pManager->unregisterTimer(this);
	}
}

sal_Bool OTimer::isTicking() const
{
	OTimerManager *pManager = OTimerManager::getTimerManager();

	VOS_ASSERT(pManager);

	if (pManager)
		return pManager->lookupTimer(this);
	else
		return sal_False;

}

sal_Bool OTimer::isExpired() const
{
	TTimeValue Now;

	osl_getSystemTime(&Now);

	return !(Now < m_Expired);
}

sal_Bool OTimer::expiresBefore(const OTimer* pTimer) const
{
	VOS_ASSERT(pTimer);

	if ( pTimer != 0 )
	{
		return m_Expired < pTimer->m_Expired;
	}
	else
	{
		return sal_False;
	}
}

void OTimer::setAbsoluteTime(const TTimeValue& Time)
{
	m_TimeOut     = 0;
	m_Expired     = Time;
	m_RepeatDelta = 0;

	m_Expired.normalize();
}

void OTimer::setRemainingTime(const TTimeValue& Remaining)
{
	osl_getSystemTime(&m_Expired);

	m_Expired.addTime(Remaining);
}

void OTimer::setRemainingTime(const TTimeValue& Remaining, const TTimeValue& Repeat)
{
	osl_getSystemTime(&m_Expired);

	m_Expired.addTime(Remaining);

	m_RepeatDelta = Repeat;
}

void OTimer::addTime(const TTimeValue& Delta)
{
	m_Expired.addTime(Delta);
}

TTimeValue OTimer::getRemainingTime() const
{
	TTimeValue Now;

	osl_getSystemTime(&Now);

	sal_Int32 secs = m_Expired.Seconds - Now.Seconds;

	if (secs < 0)
		return TTimeValue(0, 0);

	sal_Int32 nsecs = m_Expired.Nanosec - Now.Nanosec;

	if (nsecs < 0)
	{
		if (secs > 0)
		{
			secs  -= 1;
			nsecs += 1000000000;
		}
		else
			return TTimeValue(0, 0);
	}

	return TTimeValue(secs, nsecs);
}


/////////////////////////////////////////////////////////////////////////////
//
// Timer manager
//

OMutex NAMESPACE_VOS(OTimerManager)::m_Access;
OTimerManager* NAMESPACE_VOS(OTimerManager)::m_pManager=0;

OTimerManager::OTimerManager()
{
	OGuard Guard(&m_Access);

	VOS_ASSERT(m_pManager == 0);

	m_pManager = this;

	m_pHead= 0;

	m_notEmpty.reset();

	// start thread
#ifdef MACOSX
	// Fix race condition that causes crashing
	createSuspended();
	resume();
#else	// MACOSX
	create();
#endif	// MACOSX
}

OTimerManager::~OTimerManager()
{
	OGuard Guard(&m_Access);

	if ( m_pManager == this )
		m_pManager = 0;
}

void OTimerManager::onTerminated()
{
    delete this; // mfe: AAARRRGGGHHH!!!
}
    
OTimerManager* OTimerManager::getTimerManager()
{    
	OGuard Guard(&m_Access); 
	
	if (! m_pManager)
		new OTimerManager;

	return (m_pManager);
}	

sal_Bool OTimerManager::registerTimer(OTimer* pTimer)
{
	VOS_ASSERT(pTimer);

	if ( pTimer == 0 )
	{
		return sal_False;
	}
	
	OGuard Guard(&m_Lock);

	// try to find one with equal or lower remaining time.
	OTimer** ppIter = &m_pHead;
	
	while (*ppIter) 
	{
		if (pTimer->expiresBefore(*ppIter))
		{	
			// next element has higher remaining time, 
			// => insert new timer before
			break;
		}
		ppIter= &((*ppIter)->m_pNext);
	} 
  
	// next element has higher remaining time, 
	// => insert new timer before
	pTimer->m_pNext= *ppIter;
	*ppIter = pTimer;
    

	if (pTimer == m_pHead)
	{
		// it was inserted as new head
		// signal it to TimerManager Thread
        m_notEmpty.set();
	}

	return sal_True;
}

sal_Bool OTimerManager::unregisterTimer(OTimer* pTimer)
{
	VOS_ASSERT(pTimer);

	if ( pTimer == 0 )
	{
		return sal_False;
	}
	
	// lock access
	OGuard Guard(&m_Lock);

	OTimer** ppIter = &m_pHead;

	while (*ppIter) 
	{
		if (pTimer == (*ppIter))
		{	
			// remove timer from list
			*ppIter = (*ppIter)->m_pNext;
			return sal_True;
		}
		ppIter= &((*ppIter)->m_pNext);
	} 

	return sal_False;
}

sal_Bool OTimerManager::lookupTimer(const OTimer* pTimer) 
{
	VOS_ASSERT(pTimer);

	if ( pTimer == 0 )
	{
		return sal_False;
	}
	
	// lock access
	OGuard Guard(&m_Lock);

	// check the list
	for (OTimer* pIter = m_pHead; pIter != 0; pIter= pIter->m_pNext)
	{
		if (pIter == pTimer)
        {    
			return sal_True;
        }
	}

	return sal_False;
}

void OTimerManager::checkForTimeout()
{

    m_Lock.acquire();

	if ( m_pHead == 0 )
	{
        m_Lock.release();        
		return;
	}

	OTimer* pTimer = m_pHead;

#ifdef USE_JAVA
	// Fix deadlocks reported in the following NeoOffice forum topics by
	// locking the application mutex before executing the timer:
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8428
	// http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&t=8456
	Application_GetSolarMutexFunc *pFunc = (Application_GetSolarMutexFunc *)dlsym( RTLD_DEFAULT, "Application_GetSolarMutex" );
#endif	// USE_JAVA

	if (pTimer->isExpired())
	{
		// remove expired timer
		m_pHead = pTimer->m_pNext;

        pTimer->acquire();
        
		m_Lock.release();
		
#ifdef USE_JAVA
		if ( pFunc )
		{
			IMutex *pSolarMutex = pFunc();
			if ( pSolarMutex )
				pSolarMutex->acquire();
		}
#endif	// USE_JAVA
		pTimer->onShot();
#ifdef USE_JAVA
		if ( pFunc )
		{
			IMutex *pSolarMutex = pFunc();
			if ( pSolarMutex )
				pSolarMutex->release();
		}
#endif	// USE_JAVA

		// restart timer if specified
		if ( ! pTimer->m_RepeatDelta.isEmpty() )
		{
			TTimeValue Now;

			osl_getSystemTime(&Now);

			Now.Seconds += pTimer->m_RepeatDelta.Seconds;				
			Now.Nanosec += pTimer->m_RepeatDelta.Nanosec;				

			pTimer->m_Expired = Now;

			registerTimer(pTimer);
		}
        pTimer->release();
	}
	else
	{
		m_Lock.release();
	}


	return;
}

void OTimerManager::run() 
{
	setPriority(TPriority_BelowNormal);

	while (schedule())
	{
		TTimeValue		delay;
		TTimeValue*		pDelay=0;

        
		m_Lock.acquire();

		if (m_pHead != 0)
        {
			delay = m_pHead->getRemainingTime();
            pDelay=&delay;
        }
        else
        {
            pDelay=0;
        }
        
        
        m_notEmpty.reset();

        m_Lock.release();

        
        m_notEmpty.wait(pDelay);

        checkForTimeout();
  	}

}


/////////////////////////////////////////////////////////////////////////////
//
// Timer manager cleanup
//

// jbu:
// The timer manager cleanup has been removed (no thread is killed anymore).
// So the thread leaks.
// This will result in a GPF in case the vos-library gets unloaded before
// process termination.
// -> TODO : rewrite this file, so that the timerManager thread gets destroyed,
//           when there are no timers anymore !
