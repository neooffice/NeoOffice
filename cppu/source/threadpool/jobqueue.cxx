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
 * Modified January 2017 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppu.hxx"
#include "jobqueue.hxx"
#include "threadpool.hxx"

#include <osl/diagnose.h>

#if defined USE_JAVA && defined MACOSX

#include <dlfcn.h>

#include <tools/solar.h>

typedef void Application_acquireAllSolarMutexFunc( ULONG nCount );
typedef ULONG Application_releaseAllSolarMutexFunc();

#endif  // USE_JAVA && MACOSX

using namespace ::osl;

namespace cppu_threadpool {
	
	JobQueue::JobQueue() :
		m_nToDo( 0 ),
		m_bSuspended( sal_False ),
		m_cndWait( osl_createCondition() )
	{
		osl_resetCondition( m_cndWait );		
	}
	
	JobQueue::~JobQueue()
	{
		osl_destroyCondition( m_cndWait );
	}
		
		
	void JobQueue::add( void *pThreadSpecificData, RequestFun * doRequest )
	{
		MutexGuard guard( m_mutex );
		Job job = { pThreadSpecificData , doRequest };
		m_lstJob.push_back( job );
		if( ! m_bSuspended )
		{
			osl_setCondition( m_cndWait );
		}
		m_nToDo ++;
	}
		
	void *JobQueue::enter( sal_Int64 nDisposeId , sal_Bool bReturnWhenNoJob )
	{
		void *pReturn = 0;
		{
			// synchronize with the dispose calls
			MutexGuard guard( m_mutex );
			if( DisposedCallerAdmin::getInstance()->isDisposed( nDisposeId ) )
			{
				return 0;
			}
			m_lstCallstack.push_front( nDisposeId );
		}

		
#if defined USE_JAVA && defined MACOSX
		// Fix deadlocks when using Java or Python-based extensions, such as
		// cadlo, immediately after installation:
		// https://extensions.libreoffice.org/extensions/cadlo
		Application_acquireAllSolarMutexFunc *pAcquireAllFunc = (Application_acquireAllSolarMutexFunc *)dlsym( RTLD_DEFAULT, "Application_acquireAllSolarMutex" );
		Application_releaseAllSolarMutexFunc *pReleaseAllFunc = (Application_releaseAllSolarMutexFunc *)dlsym( RTLD_DEFAULT, "Application_releaseAllSolarMutex" );
#endif  // USE_JAVA && MACOSX

		while( sal_True )
		{
			if( bReturnWhenNoJob )
			{
				MutexGuard guard( m_mutex );
				if( m_lstJob.empty() )
				{
					break;
				}
			}

#if defined USE_JAVA && defined MACOSX
			ULONG nCount = 0;
			if ( pAcquireAllFunc && pReleaseAllFunc )
				nCount = pReleaseAllFunc();
#endif  // USE_JAVA && MACOSX
			osl_waitCondition( m_cndWait , 0 );
#if defined USE_JAVA && defined MACOSX
			if ( pAcquireAllFunc && pReleaseAllFunc )
				pAcquireAllFunc( nCount );
#endif  // USE_JAVA && MACOSX
			
			struct Job job={0,0};
			{
				// synchronize with add and dispose calls
				MutexGuard guard( m_mutex );

				if( 0 == m_lstCallstack.front() )
				{
					// disposed !
					break;
				}

				OSL_ASSERT( ! m_lstJob.empty() );
				if( ! m_lstJob.empty() )
				{
					job = m_lstJob.front();
					m_lstJob.pop_front();
				}
				if( m_lstJob.empty() )
				{
					osl_resetCondition( m_cndWait );
				}
			}

			if( job.doRequest )
			{
				job.doRequest( job.pThreadSpecificData );
				m_nToDo --;
			}
			else
			{
				m_nToDo --;
				pReturn = job.pThreadSpecificData;
				break;
			}
		}

		{
			// synchronize with the dispose calls
			MutexGuard guard( m_mutex );
			m_lstCallstack.pop_front();
		}
		
		return pReturn;
	}
	
	void JobQueue::dispose( sal_Int64 nDisposeId )
	{
		MutexGuard guard( m_mutex );
		for( CallStackList::iterator ii = m_lstCallstack.begin() ;
			 ii != m_lstCallstack.end() ;
			 ++ii )
		{
			if( (*ii) == nDisposeId )
			{
				(*ii) = 0;
			}
		}

		if( !m_lstCallstack.empty()  && ! m_lstCallstack.front() )
		{
			// The thread is waiting for a disposed pCallerId, let it go
			osl_setCondition( m_cndWait );
		}
	}
		
	void JobQueue::suspend()
	{
		MutexGuard guard( m_mutex );
		m_bSuspended = sal_True;
	}

	void JobQueue::resume()
	{
		MutexGuard guard( m_mutex );
		m_bSuspended = sal_False;
		if( ! m_lstJob.empty() )
		{
			osl_setCondition( m_cndWait );
		}
	}

	sal_Bool JobQueue::isEmpty()
	{
		MutexGuard guard( m_mutex );
		return m_lstJob.empty();
	}

	sal_Bool JobQueue::isCallstackEmpty()
	{
		MutexGuard guard( m_mutex );
		return m_lstCallstack.empty();
	}

	sal_Bool JobQueue::isBusy()
	{
		return m_nToDo > 0;
	}


}
