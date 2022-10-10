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

#include <svtools/svtools.hrc>
#include <tools/resid.hxx>
#include <com/sun/star/task/XInteractionContinuation.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>
#include <com/sun/star/java/JavaNotFoundException.hpp>
#include <com/sun/star/java/InvalidJavaSettingsException.hpp>
#include <com/sun/star/java/JavaDisabledException.hpp>
#include <com/sun/star/java/JavaVMCreationFailureException.hpp>
#include <com/sun/star/java/RestartRequiredException.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/layout.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <tools/rcid.h>
#include <jvmfwk/framework.h>

#include <svtools/restartdialog.hxx>
#include <svtools/svtresid.hxx>
#include <svtools/javainteractionhandler.hxx>
#include <svtools/javacontext.hxx>

#if defined PRODUCT_JAVA_DOWNLOAD_URL && defined USE_JAVA && defined MACOSX

#include <vcl/msgbox.hxx>
#import "javainteractionhandler_cocoa.h"
#include "../../../extensions/source/update/check/updatehdl.hrc"

static ResMgr *pUpdResMgr = NULL;

#endif	// PRODUCT_JAVA_DOWNLOAD_URL && USE_JAVA && MACOSX

using namespace com::sun::star::uno;
using namespace com::sun::star::task;

namespace svt
{

#if defined PRODUCT_JAVA_DOWNLOAD_URL && defined USE_JAVA && defined MACOSX

static OUString GetUpdResString( int nId )
{
    if ( !pUpdResMgr )
    {
        pUpdResMgr = ResMgr::CreateResMgr( "upd" );
        if ( !pUpdResMgr )
            return "";
    }

    ResId aResId( nId, *pUpdResMgr );
    aResId.SetRT( RSC_STRING );
    if ( !pUpdResMgr->IsAvailable( aResId ) )
        return "";
 
    return OUString( ResId( nId, *pUpdResMgr ) );
}

#endif	// PRODUCT_JAVA_DOWNLOAD_URL && USE_JAVA && MACOSX

JavaInteractionHandler::JavaInteractionHandler(bool bReportErrorOnce) :
    m_aRefCount(0),
    m_bShowErrorsOnce(bReportErrorOnce),
    m_bJavaDisabled_Handled(false),
    m_bInvalidSettings_Handled(false),
    m_bJavaNotFound_Handled(false),
    m_bVMCreationFailure_Handled(false),
    m_bRestartRequired_Handled(false),
    m_nResult_JavaDisabled(RET_NO)
{
}

JavaInteractionHandler::~JavaInteractionHandler()
{
}

Any SAL_CALL JavaInteractionHandler::queryInterface(const Type& aType )
    throw (RuntimeException, std::exception)
{
    if (aType == cppu::UnoType<XInterface>::get())
        return Any( static_cast<XInterface*>(this), aType);
    else if (aType == cppu::UnoType<XInteractionHandler>::get())
        return Any( static_cast<XInteractionHandler*>(this), aType);
    return Any();
}

void SAL_CALL JavaInteractionHandler::acquire(  ) throw ()
{
    osl_atomic_increment( &m_aRefCount );
}

void SAL_CALL JavaInteractionHandler::release(  ) throw ()
{
    if (! osl_atomic_decrement( &m_aRefCount ))
        delete this;
}


void SAL_CALL JavaInteractionHandler::handle( const Reference< XInteractionRequest >& Request ) throw (RuntimeException, std::exception)
{
    Any anyExc = Request->getRequest();
    Sequence< Reference< XInteractionContinuation > > aSeqCont = Request->getContinuations();

    Reference< XInteractionAbort > abort;
    Reference< XInteractionRetry > retry;
    sal_Int32 i;

    for ( i = 0; i < aSeqCont.getLength(); i++ )
    {
        abort = Reference< XInteractionAbort>::query( aSeqCont[i]);
        if ( abort.is() )
            break;
    }

    for ( i= 0; i < aSeqCont.getLength(); i++)
    {
        retry= Reference<XInteractionRetry>::query( aSeqCont[i]);
        if ( retry.is() )
            break;
    }

    com::sun::star::java::JavaNotFoundException e1;
    com::sun::star::java::InvalidJavaSettingsException e2;
     com::sun::star::java::JavaDisabledException                e3;
    com::sun::star::java::JavaVMCreationFailureException    e4;
    com::sun::star::java::RestartRequiredException e5;
    // Try to recover the Exception type in the any and
    // react accordingly.
    sal_uInt16      nResult = RET_CANCEL;

    if ( anyExc >>= e1 )
    {
        if( ! (m_bShowErrorsOnce && m_bJavaNotFound_Handled))
        {
           // No suitable JRE found
            SolarMutexGuard aSolarGuard;
            m_bJavaNotFound_Handled = true;
            MessageDialog aWarningBox(NULL, SvtResId(STR_WARNING_JAVANOTFOUND), VCL_MESSAGE_WARNING);
            aWarningBox.SetText(SvtResId(STR_WARNING_JAVANOTFOUND_TITLE));
#if defined PRODUCT_JAVA_DOWNLOAD_URL && defined USE_JAVA && defined MACOSX
            OUString aDownload = GetUpdResString(RID_UPDATE_BTN_DOWNLOAD);
            aDownload = aDownload.replaceAll("~", "");
            if (aDownload.getLength())
            {
                QueryBox aQueryBox(NULL, WB_OK_CANCEL | WB_DEF_OK, SvtResId(STR_WARNING_JAVANOTFOUND));
                aQueryBox.SetText(aWarningBox.GetText());
                aQueryBox.SetButtonText(RET_OK, aDownload);
                nResult = aQueryBox.Execute();
                if (nResult == RET_OK)
                    JavaInteractionHandler_downloadJava();
            }
            else
            {
#endif	// PRODUCT_JAVA_DOWNLOAD_URL && USE_JAVA && MACOSX
            nResult = aWarningBox.Execute();
#if defined PRODUCT_JAVA_DOWNLOAD_URL && defined USE_JAVA && defined MACOSX
            }
#endif	// PRODUCT_JAVA_DOWNLOAD_URL && USE_JAVA && MACOSX
        }
        else
        {
            nResult = RET_OK;
        }
    }
    else if ( anyExc >>= e2 )
    {
        if( !(m_bShowErrorsOnce && m_bInvalidSettings_Handled))
        {
           // javavendors.xml was updated and Java has not been configured yet
            SolarMutexGuard aSolarGuard;
            m_bInvalidSettings_Handled = true;
#ifdef MACOSX
            MessageDialog aWarningBox(NULL, SvtResId(STR_WARNING_INVALIDJAVASETTINGS_MAC), VCL_MESSAGE_WARNING);
#else
            MessageDialog aWarningBox(NULL, SvtResId(STR_WARNING_INVALIDJAVASETTINGS), VCL_MESSAGE_WARNING);
#endif
            aWarningBox.SetText(SvtResId(STR_WARNING_INVALIDJAVASETTINGS_TITLE));
#if defined PRODUCT_JAVA_DOWNLOAD_URL && defined USE_JAVA && defined MACOSX
            OUString aDownload = GetUpdResString(RID_UPDATE_BTN_DOWNLOAD);
            aDownload = aDownload.replaceAll("~", "");
            if (aDownload.getLength())
            {
                QueryBox aQueryBox(NULL, WB_OK_CANCEL | WB_DEF_OK, SvtResId(STR_WARNING_INVALIDJAVASETTINGS_TITLE));
                aQueryBox.SetText(aWarningBox.GetText());
                aQueryBox.SetButtonText(RET_OK, aDownload);
                nResult = aQueryBox.Execute();
                if (nResult == RET_OK)
                    JavaInteractionHandler_downloadJava();
            }
            else
            {
#endif	// PRODUCT_JAVA_DOWNLOAD_URL && USE_JAVA && MACOSX
            nResult = aWarningBox.Execute();
#if defined PRODUCT_JAVA_DOWNLOAD_URL && defined USE_JAVA && defined MACOSX
            }
#endif	// PRODUCT_JAVA_DOWNLOAD_URL && USE_JAVA && MACOSX
        }
        else
        {
            nResult = RET_OK;
        }
    }
    else if ( anyExc >>= e3 )
    {
        if( !(m_bShowErrorsOnce && m_bJavaDisabled_Handled))
        {
            SolarMutexGuard aSolarGuard;
            m_bJavaDisabled_Handled = true;
            // Java disabled. Give user a chance to enable Java inside Office.
            MessageDialog aQueryBox(NULL, "JavaDisabledDialog",
                                    "svt/ui/javadisableddialog.ui");
            nResult = aQueryBox.Execute();
            if ( nResult == RET_YES )
            {
                jfw_setEnabled(sal_True);
            }

            m_nResult_JavaDisabled = nResult;

        }
        else
        {
            nResult = m_nResult_JavaDisabled;
        }
    }
    else if ( anyExc >>= e4 )
    {
        if( !(m_bShowErrorsOnce && m_bVMCreationFailure_Handled))
        {
            // Java not correctly installed, or damaged
            SolarMutexGuard aSolarGuard;
            m_bVMCreationFailure_Handled = true;
#ifdef MACOSX
            MessageDialog aErrorBox(NULL, SvtResId(STR_ERROR_JVMCREATIONFAILED_MAC));
#else
            MessageDialog aErrorBox(NULL, SvtResId(STR_ERROR_JVMCREATIONFAILED));
#endif
            aErrorBox.SetText(SvtResId(STR_ERROR_JVMCREATIONFAILED_TITLE));
#if defined PRODUCT_JAVA_DOWNLOAD_URL && defined USE_JAVA && defined MACOSX
            OUString aDownload = GetUpdResString(RID_UPDATE_BTN_DOWNLOAD);
            aDownload = aDownload.replaceAll("~", "");
            if (aDownload.getLength())
            {
                QueryBox aQueryBox(NULL, WB_OK_CANCEL | WB_DEF_OK, SvtResId(STR_ERROR_JVMCREATIONFAILED));
                aQueryBox.SetText(aErrorBox.GetText());
                aQueryBox.SetButtonText(RET_OK, aDownload);
                nResult = aQueryBox.Execute();
                if (nResult == RET_OK)
                    JavaInteractionHandler_downloadJava();
            }
            else
            {
#endif	// PRODUCT_JAVA_DOWNLOAD_URL && USE_JAVA && MACOSX
            nResult = aErrorBox.Execute();
#if defined PRODUCT_JAVA_DOWNLOAD_URL && defined USE_JAVA && defined MACOSX
            }
#endif	// PRODUCT_JAVA_DOWNLOAD_URL && USE_JAVA && MACOSX
        }
        else
        {
            nResult = RET_OK;
        }
    }
    else if ( anyExc >>= e5 )
    {
        if( !(m_bShowErrorsOnce && m_bRestartRequired_Handled))
        {
            // a new JRE was selected, but office needs to be restarted
            //before it can be used.
            SolarMutexGuard aSolarGuard;
            m_bRestartRequired_Handled = true;
            svtools::executeRestartDialog(
                comphelper::getProcessComponentContext(), 0,
                svtools::RESTART_REASON_JAVA);
        }
        nResult = RET_OK;
    }

    if ( nResult == RET_CANCEL || nResult == RET_NO)
    {
        // Unknown exception type or user wants to cancel
        if ( abort.is() )
            abort->select();
    }
    else // nResult == RET_OK
    {
        // User selected OK => retry Java usage
        if ( retry.is() )
            retry->select();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
