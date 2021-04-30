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

#include <config_folders.h>

#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/process.h>
#include <osl/file.hxx>
#include <rtl/ustrbuf.hxx>

#include <rtl/uri.hxx>
#include "shellexec.hxx"
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>

#include <com/sun/star/util/theMacroExpander.hpp>
#include <com/sun/star/uri/XExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <cppuhelper/supportsservice.hxx>

#include "uno/current_context.hxx"

#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifndef NO_LIBO_OPEN_EXECUTABLE_FIX
#if defined MACOSX
#include <sys/stat.h>
#endif
#endif	// !NO_LIBO_OPEN_EXECUTABLE_FIX

#if defined USE_JAVA && defined MACOSX

#include <dlfcn.h>

#import "shellexec_cocoa.h"

typedef void* id;
typedef id Application_acquireSecurityScopedURLFromOUString_Type( const OUString *pNonSecurityScopedURL, unsigned char bMustShowDialogIfNoBookmark, const OUString *pDialogTitle );
typedef void Application_releaseSecurityScopedURL_Type( id pSecurityScopedURLs );

static Application_acquireSecurityScopedURLFromOUString_Type *pApplication_acquireSecurityScopedURLFromOUString = NULL;
static Application_releaseSecurityScopedURL_Type *pApplication_releaseSecurityScopedURL = NULL;

#endif	// USE_JAVA && MACOSX


// namespace directives


using com::sun::star::system::XSystemShellExecute;
using com::sun::star::system::SystemShellExecuteException;

using osl::FileBase;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::system::SystemShellExecuteFlags;
using namespace cppu;

#define SHELLEXEC_IMPL_NAME  "com.sun.star.comp.system.SystemShellExecute2"


// helper functions


namespace // private
{
    Sequence< OUString > SAL_CALL ShellExec_getSupportedServiceNames()
    {
        Sequence< OUString > aRet(1);
        aRet[0] = "com.sun.star.sys.shell.SystemShellExecute";
        return aRet;
    }
}

void escapeForShell( OStringBuffer & rBuffer, const OString & rURL)
{
    sal_Int32 nmax = rURL.getLength();
    for(sal_Int32 n=0; n < nmax; ++n)
    {
        // escape every non alpha numeric characters (excluding a few "known good") by prepending a '\'
        sal_Char c = rURL[n];
        if( ( c < 'A' || c > 'Z' ) && ( c < 'a' || c > 'z' ) && ( c < '0' || c > '9' )  && c != '/' && c != '.' )
            rBuffer.append( '\\' );

        rBuffer.append( c );
    }
}



ShellExec::ShellExec( const Reference< XComponentContext >& xContext ) :
    WeakImplHelper2< XSystemShellExecute, XServiceInfo >(),
    m_xContext(xContext)
{
    try {
        Reference< XCurrentContext > xCurrentContext(getCurrentContext());

        if (xCurrentContext.is())
        {
            Any aValue = xCurrentContext->getValueByName(
                OUString( "system.desktop-environment"  ) );

            OUString aDesktopEnvironment;
            if (aValue >>= aDesktopEnvironment)
            {
                m_aDesktopEnvironment = OUStringToOString(aDesktopEnvironment, RTL_TEXTENCODING_ASCII_US);
            }
        }
    } catch (const RuntimeException &) {
    }
}



void SAL_CALL ShellExec::execute( const OUString& aCommand, const OUString& aParameter, sal_Int32 nFlags )
    throw (IllegalArgumentException, SystemShellExecuteException, RuntimeException, std::exception)
{
#if defined USE_JAVA && defined MACOSX
    OStringBuffer aBuffer;
#else	// USE_JAVA && MACOSX
    OStringBuffer aBuffer, aLaunchBuffer;

    // DESKTOP_LAUNCH, see http://freedesktop.org/pipermail/xdg/2004-August/004489.html
    static const char *pDesktopLaunch = getenv( "DESKTOP_LAUNCH" );
#endif	// USE_JAVA && MACOSX

    // Check whether aCommand contains an absolute URI reference:
    css::uno::Reference< css::uri::XUriReference > uri(
        css::uri::UriReferenceFactory::create(m_xContext)->parse(aCommand));
    if (uri.is() && uri->isAbsolute())
    {
        // It seems to be a url ..
        // We need to re-encode file urls because osl_getFileURLFromSystemPath converts
        // to UTF-8 before encoding non ascii characters, which is not what other apps
        // expect.
        OUString aURL(
            com::sun::star::uri::ExternalUriReferenceTranslator::create(
                m_xContext)->translateToExternal(aCommand));
        if ( aURL.isEmpty() && !aCommand.isEmpty() )
        {
            throw RuntimeException(
                OUString("Cannot translate URI reference to external format: ")
                 + aCommand,
                static_cast< cppu::OWeakObject * >(this));
        }

#ifdef MACOSX
#ifndef NO_LIBO_OPEN_EXECUTABLE_FIX
        bool dir = false;
        if (uri->getScheme().equalsIgnoreAsciiCase("file")) {
            OUString pathname;
            auto const e1 = osl::FileBase::getSystemPathFromFileURL(aCommand, pathname);
            if (e1 != osl::FileBase::E_None) {
                throw css::lang::IllegalArgumentException(
                    ("XSystemShellExecute.execute, getSystemPathFromFileURL <" + aCommand
                     + "> failed with " + OUString::number(e1)),
                    {}, 0);
            }
            OString pathname8;
            if (!pathname.convertToString(
                    &pathname8, RTL_TEXTENCODING_UTF8,
                    (RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR
                     | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR)))
            {
                throw css::lang::IllegalArgumentException(
                    "XSystemShellExecute.execute, cannot convert \"" + pathname + "\" to UTF-8", {},
                    0);
            }
            struct stat st;
            auto const e2 = stat(pathname8.getStr(), &st);
            if (e2 != 0) {
                auto const e3 = errno;
                SAL_INFO("shell", "stat(" << pathname8 << ") failed with errno " << e3);
            }
            if (e2 == 0 && S_ISDIR(st.st_mode)) {
                dir = true;
            } else if (e2 != 0 || !S_ISREG(st.st_mode)
                       || (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0)
            {
                throw css::lang::IllegalArgumentException(
                    "XSystemShellExecute.execute, cannot process <" + aCommand + ">", {}, 0);
            } else if (pathname.endsWithIgnoreAsciiCase(".class")
                       || pathname.endsWithIgnoreAsciiCase(".fileloc")
                       || pathname.endsWithIgnoreAsciiCase(".jar"))
            {
                dir = true;
            }
        }
#endif	// !NO_LIBO_OPEN_EXECUTABLE_FIX

#ifdef USE_JAVA
        // Fix failure to open file URL hyperlinks by obtaining a security
        // scoped bookmark before opening the URL
        id pSecurityScopedURL = NULL;
        if ( 0 == aURL.startsWith("file://") )
        {
            if ( !pApplication_acquireSecurityScopedURLFromOUString )
                pApplication_acquireSecurityScopedURLFromOUString = (Application_acquireSecurityScopedURLFromOUString_Type *)dlsym( RTLD_DEFAULT, "Application_acquireSecurityScopedURLFromOUString" );
            if ( !pApplication_releaseSecurityScopedURL )
                pApplication_releaseSecurityScopedURL = (Application_releaseSecurityScopedURL_Type *)dlsym( RTLD_DEFAULT, "Application_releaseSecurityScopedURL" );
            if ( pApplication_acquireSecurityScopedURLFromOUString && pApplication_releaseSecurityScopedURL )
                pSecurityScopedURL = pApplication_acquireSecurityScopedURLFromOUString( &aURL, sal_True, NULL );
        }

        // Fix bug 3584 by not throwing an exception if we can't open a URL
        sal_Bool bOpened = ShellExec_openURL( aURL, dir );

        if ( pSecurityScopedURL && pApplication_releaseSecurityScopedURL )
            pApplication_releaseSecurityScopedURL( pSecurityScopedURL );

        if ( !bOpened )
            throw SystemShellExecuteException( "Cound not open path",
                static_cast < XSystemShellExecute * > (this), ENOENT );

        return;
#else	// USE_JAVA
        //TODO: Using open(1) with an argument that syntactically is an absolute
        // URI reference does not necessarily give expected results:
        // 1  If the given URI reference matches a supported scheme (e.g.,
        //  "mailto:foo"):
        // 1.1  If it matches an existing pathname (relative to CWD):  Results
        //  in "mailto:foo?\n[0]\tcancel\n[1]\tOpen the file\tmailto:foo\n[2]\t
        //  Open the URL\tmailto:foo\n\nWhich did you mean? Cancelled." on
        //  stderr and SystemShellExecuteException.
        // 1.2  If it does not match an exitsting pathname (relative to CWD):
        //  Results in the corresponding application being opened with the given
        //  document (e.g., Mail with a New Message).
        // 2  If the given URI reference does not match a supported scheme
        //  (e.g., "foo:bar"):
        // 2.1  If it matches an existing pathname (relative to CWD) pointing to
        //  an executable:  Results in execution of that executable.
        // 2.2  If it matches an existing pathname (relative to CWD) pointing to
        //  a non-executable regular file:  Results in opening it in TextEdit.
        // 2.3  If it matches an existing pathname (relative to CWD) pointing to
        //  a directory:  Results in opening it in Finder.
        // 2.4  If it does not match an exitsting pathname (relative to CWD):
        //  Results in "The file /.../foo:bar does not exits." (where "/..." is
        //  the CWD) on stderr and SystemShellExecuteException.
        aBuffer.append("open");
        if (dir) {
            aBuffer.append(" -R");
        }
        aBuffer.append(" --");
#endif	// USE_JAVA
#else
        // The url launchers are expected to be in the $BRAND_BASE_DIR/LIBO_LIBEXEC_FOLDER
        // directory:
        com::sun::star::uno::Reference< com::sun::star::util::XMacroExpander >
            exp = com::sun::star::util::theMacroExpander::get(m_xContext);
        OUString aProgramURL;
        try {
            aProgramURL = exp->expandMacros(
                OUString( "$BRAND_BASE_DIR/" LIBO_LIBEXEC_FOLDER "/"));
        } catch (com::sun::star::lang::IllegalArgumentException &)
        {
            throw SystemShellExecuteException(
                "Could not expand $BRAND_BASE_DIR path",
                static_cast < XSystemShellExecute * > (this), ENOENT );
        }

        OUString aProgram;
        if ( FileBase::E_None != FileBase::getSystemPathFromFileURL(aProgramURL, aProgram))
        {
            throw SystemShellExecuteException(
                "Cound not convert executable path",
                static_cast < XSystemShellExecute * > (this), ENOENT );
        }

        OString aTmp = OUStringToOString(aProgram, osl_getThreadTextEncoding());
        escapeForShell(aBuffer, aTmp);

#ifdef SOLARIS
        if ( m_aDesktopEnvironment.getLength() == 0 )
             m_aDesktopEnvironment = OString("GNOME");
#endif

        // Respect the desktop environment - if there is an executable named
        // <desktop-environement-is>-open-url, pass the url to this one instead
        // of the default "open-url" script.
        if ( !m_aDesktopEnvironment.isEmpty() )
        {
            OString aDesktopEnvironment(m_aDesktopEnvironment.toAsciiLowerCase());
            OStringBuffer aCopy(aTmp);

            aCopy.append(aDesktopEnvironment + "-open-url");

            if ( 0 == access( aCopy.getStr(), X_OK) )
            {
                aBuffer.append(aDesktopEnvironment + "-");
            }
        }

        aBuffer.append("open-url");
#endif
#if !defined USE_JAVA || !defined MACOSX
        aBuffer.append(" ");
        escapeForShell(aBuffer, OUStringToOString(aURL, osl_getThreadTextEncoding()));

        if ( pDesktopLaunch && *pDesktopLaunch )
        {
            aLaunchBuffer.append( OString(pDesktopLaunch) + " ");
            escapeForShell(aLaunchBuffer, OUStringToOString(aURL, osl_getThreadTextEncoding()));
        }
#endif	// !USE_JAVA || !MACOSX
    } else if ((nFlags & css::system::SystemShellExecuteFlags::URIS_ONLY) != 0)
    {
        throw css::lang::IllegalArgumentException(
            OUString("XSystemShellExecute.execute URIS_ONLY with non-absolute"
                     " URI reference ")
             + aCommand,
            static_cast< cppu::OWeakObject * >(this), 0);
    } else {
        escapeForShell(aBuffer, OUStringToOString(aCommand, osl_getThreadTextEncoding()));
        aBuffer.append(" ");
        if( nFlags != 42 )
            escapeForShell(aBuffer, OUStringToOString(aParameter, osl_getThreadTextEncoding()));
        else
            aBuffer.append(OUStringToOString(aParameter, osl_getThreadTextEncoding()));
    }

#ifndef USE_JAVA
    // Prefer DESKTOP_LAUNCH when available
    if ( !aLaunchBuffer.isEmpty() )
    {
        FILE *pLaunch = popen( aLaunchBuffer.makeStringAndClear().getStr(), "w" );
        if ( pLaunch != NULL )
        {
            if ( 0 == pclose( pLaunch ) )
                return;
        }
        // Failed, do not try DESKTOP_LAUNCH any more
        pDesktopLaunch = NULL;
    }
#endif	// !USE_JAVA

    OString cmd =
#ifdef LINUX
        // avoid blocking (call it in background)
        "( " + aBuffer.makeStringAndClear() +  " ) &";
#else
        aBuffer.makeStringAndClear();
#endif
    FILE *pLaunch = popen(cmd.getStr(), "w");
    if ( pLaunch != NULL )
    {
        if ( 0 == pclose( pLaunch ) )
            return;
    }

    int nerr = errno;
    throw SystemShellExecuteException(OUString::createFromAscii( strerror( nerr ) ),
        static_cast < XSystemShellExecute * > (this), nerr );
}

// XServiceInfo
OUString SAL_CALL ShellExec::getImplementationName(  )
    throw( RuntimeException, std::exception )
{
    return OUString(SHELLEXEC_IMPL_NAME );
}

//  XServiceInfo
sal_Bool SAL_CALL ShellExec::supportsService( const OUString& ServiceName )
    throw( RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

//  XServiceInfo
Sequence< OUString > SAL_CALL ShellExec::getSupportedServiceNames(   )
    throw( RuntimeException, std::exception )
{
    return ShellExec_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
