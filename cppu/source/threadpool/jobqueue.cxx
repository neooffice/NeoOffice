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
 *   Modified January 2017 by Patrick Luby. NeoOffice is only distributed
 *   under the GNU General Public License, Version 3 as allowed by Section 3.3
 *   of the Mozilla Public License, v. 2.0.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "jobqueue.hxx"
#include "threadpool.hxx"

#include <osl/diagnose.h>

#if defined USE_JAVA && defined MACOSX

#include <dlfcn.h>

#include <tools/solar.h>

typedef void Application_acquireAllSolarMutexFunc( sal_uLong nCount );
typedef sal_uLong Application_releaseAllSolarMutexFunc();

#endif  // USE_JAVA && MACOSX

using namespace ::osl;

namespace cppu_threadpool {

    JobQueue::JobQueue() :
        m_nToDo( 0 ),
        m_bSuspended( false ),
        m_cndWait( osl_createCondition() )
    {
        osl_resetCondition( m_cndWait );
        m_DisposedCallerAdmin = DisposedCallerAdmin::getInstance();
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

    void *JobQueue::enter( sal_Int64 nDisposeId , bool bReturnWhenNoJob )
    {
        void *pReturn = 0;
        {
            // synchronize with the dispose calls
            MutexGuard guard( m_mutex );
            if( m_DisposedCallerAdmin->isDisposed( nDisposeId ) )
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

        while( true )
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
            sal_uLong nCount = 0;
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
                    if( m_lstJob.empty()
                        && (m_lstCallstack.empty()
                            || m_lstCallstack.front() != 0) )
                    {
                        osl_resetCondition( m_cndWait );
                    }
                    break;
                }

                OSL_ASSERT( ! m_lstJob.empty() );
                if( ! m_lstJob.empty() )
                {
                    job = m_lstJob.front();
                    m_lstJob.pop_front();
                }
                if( m_lstJob.empty()
                    && (m_lstCallstack.empty() || m_lstCallstack.front() != 0) )
                {
                    osl_resetCondition( m_cndWait );
                }
            }

            if( job.doRequest )
            {
                job.doRequest( job.pThreadSpecificData );
                MutexGuard guard( m_mutex );
                m_nToDo --;
            }
            else
            {
                pReturn = job.pThreadSpecificData;
                MutexGuard guard( m_mutex );
                m_nToDo --;
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
        m_bSuspended = true;
    }

    void JobQueue::resume()
    {
        MutexGuard guard( m_mutex );
        m_bSuspended = false;
        if( ! m_lstJob.empty() )
        {
            osl_setCondition( m_cndWait );
        }
    }

    bool JobQueue::isEmpty() const
    {
        MutexGuard guard( m_mutex );
        return m_lstJob.empty();
    }

    bool JobQueue::isCallstackEmpty() const
    {
        MutexGuard guard( m_mutex );
        return m_lstCallstack.empty();
    }

    bool JobQueue::isBusy() const
    {
        MutexGuard guard( m_mutex );
        return m_nToDo > 0;
    }


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
