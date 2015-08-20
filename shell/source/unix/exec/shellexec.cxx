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
 * Modified January 2010 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"
#include <osl/diagnose.h>
#include <osl/thread.h>
#include <osl/process.h>
#include <osl/file.hxx>
#include <rtl/ustrbuf.hxx>

#ifndef _RTL_URI_H_
#include <rtl/uri.hxx>
#endif
#include "shellexec.hxx"
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>

#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/uri/XExternalUriReferenceTranslator.hpp>
#include <com/sun/star/uri/ExternalUriReferenceTranslator.hpp>

#include "uno/current_context.hxx"

#include <string.h>
#include <errno.h>
#include <unistd.h>

#if defined USE_JAVA && defined MACOSX

#include <dlfcn.h>

#import "shellexec_cocoa.h"

typedef void* id;
typedef id Application_acquireSecurityScopedURLFromOUString_Type( const ::rtl::OUString *pNonSecurityScopedURL, unsigned char bMustShowDialogIfNoBookmark, const ::rtl::OUString *pDialogTitle );
typedef void Application_releaseSecurityScopedURL_Type( id pSecurityScopedURLs );

static Application_acquireSecurityScopedURLFromOUString_Type *pApplication_acquireSecurityScopedURLFromOUString = NULL;
static Application_releaseSecurityScopedURL_Type *pApplication_releaseSecurityScopedURL = NULL;

#endif	// USE_JAVA && MACOSX

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using com::sun::star::system::XSystemShellExecute;
using com::sun::star::system::SystemShellExecuteException;

using rtl::OString;
using rtl::OUString;
using rtl::OStringBuffer;
using rtl::OUStringBuffer;
using osl::FileBase;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::system::SystemShellExecuteFlags;
using namespace cppu;

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define SHELLEXEC_IMPL_NAME  "com.sun.star.comp.system.SystemShellExecute2"

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace // private
{
    Sequence< OUString > SAL_CALL ShellExec_getSupportedServiceNames()
    {
        Sequence< OUString > aRet(1);
        aRet[0] = OUString::createFromAscii("com.sun.star.sys.shell.SystemShellExecute");
        return aRet;
    }
}

void escapeForShell( rtl::OStringBuffer & rBuffer, const rtl::OString & rURL)
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

//-----------------------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------------------

ShellExec::ShellExec( const Reference< XComponentContext >& xContext ) : 
    WeakImplHelper2< XSystemShellExecute, XServiceInfo >(),
    m_xContext(xContext)
{
    try {
        Reference< XCurrentContext > xCurrentContext(getCurrentContext());
        
        if (xCurrentContext.is())
        {
            Any aValue = xCurrentContext->getValueByName(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "system.desktop-environment" ) ) );
        
            OUString aDesktopEnvironment;
            if (aValue >>= aDesktopEnvironment)
            {
                m_aDesktopEnvironment = OUStringToOString(aDesktopEnvironment, RTL_TEXTENCODING_ASCII_US);
            }
        }
    } catch (RuntimeException e) {
    }
}

//-------------------------------------------------
//
//-------------------------------------------------

void SAL_CALL ShellExec::execute( const OUString& aCommand, const OUString& aParameter, sal_Int32 nFlags ) 
    throw (IllegalArgumentException, SystemShellExecuteException, RuntimeException)
{
    OStringBuffer aBuffer, aLaunchBuffer;

    // DESKTOP_LAUNCH, see http://freedesktop.org/pipermail/xdg/2004-August/004489.html
    static const char *pDesktopLaunch = getenv( "DESKTOP_LAUNCH" );
    
    // Check wether aCommand contains a document url or not
    sal_Int32 nIndex = aCommand.indexOf( OUString( RTL_CONSTASCII_USTRINGPARAM(":/") ) );
    
    if( nIndex > 0 || 0 == aCommand.compareToAscii("mailto:", 7) )
    {
        // It seems to be a url ..
        // We need to re-encode file urls because osl_getFileURLFromSystemPath converts
        // to UTF-8 before encoding non ascii characters, which is not what other apps
        // expect.
        OUString aURL(
            com::sun::star::uri::ExternalUriReferenceTranslator::create(
                m_xContext)->translateToExternal(aCommand));
        if ( aURL.getLength() == 0 && aCommand.getLength() != 0 )
        {
            throw RuntimeException(
                (OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "Cannot translate URI reference to external format: "))
                 + aCommand),
                static_cast< cppu::OWeakObject * >(this));
        }
        
#ifdef USE_JAVA
        // Replace ".go-oo.org" with ".services.openoffice.org" since go-oo.org
        // is now dead and the Go-oo code merely redirected to matching
        // services.openoffice.org URLs
        static const OUString aGoOOExtensions( RTL_CONSTASCII_USTRINGPARAM( "http://extensions.go-oo.org" ) );
        static const OUString aOOoExtensions( RTL_CONSTASCII_USTRINGPARAM( "http://extensions.services.openoffice.org" ) );
        static const OUString aGoOOTemplates( RTL_CONSTASCII_USTRINGPARAM( "http://templates.go-oo.org" ) );
        static const OUString aOOoTemplates( RTL_CONSTASCII_USTRINGPARAM( "http://templates.services.openoffice.org" ) );
        static const OUString aGoOOWWW( RTL_CONSTASCII_USTRINGPARAM( "http://www.go-oo.org" ) );
        static const OUString aOOoWWW( RTL_CONSTASCII_USTRINGPARAM( "http://www.openoffice.org" ) );

        if ( aURL.indexOf( aGoOOExtensions ) == 0 )
            aURL = aURL.replaceAt( 0, aGoOOExtensions.getLength(), aOOoExtensions );
        else if ( aURL.indexOf( aGoOOTemplates ) == 0 )
            aURL = aURL.replaceAt( 0, aGoOOTemplates.getLength(), aOOoTemplates );
        else if ( aURL.indexOf( aGoOOWWW ) == 0 )
            aURL = aURL.replaceAt( 0, aGoOOWWW.getLength(), aOOoWWW );

        if ( aURL.getLength() == 0 && aCommand.getLength() != 0 )
        {
            throw RuntimeException(
                (OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "Cannot replace Go-oo domain with OOo domain in URL: "))
                 + aCommand),
                static_cast< cppu::OWeakObject * >(this));
        }
#endif	// USE_JAVA

#if defined USE_JAVA && defined MACOSX
        // Fix failure to open file URL hyperlinks by obtaining a security
        // scoped bookmark before opening the URL
        id pSecurityScopedURL = NULL;
        if ( 0 == aURL.compareToAscii("file://", 7) )
        {
            if ( !pApplication_acquireSecurityScopedURLFromOUString )
                pApplication_acquireSecurityScopedURLFromOUString = (Application_acquireSecurityScopedURLFromOUString_Type *)dlsym( RTLD_DEFAULT, "Application_acquireSecurityScopedURLFromOUString" );
            if ( !pApplication_releaseSecurityScopedURL )
                pApplication_releaseSecurityScopedURL = (Application_releaseSecurityScopedURL_Type *)dlsym( RTLD_DEFAULT, "Application_releaseSecurityScopedURL" );
            if ( pApplication_acquireSecurityScopedURLFromOUString && pApplication_releaseSecurityScopedURL )
                pSecurityScopedURL = pApplication_acquireSecurityScopedURLFromOUString( &aURL, sal_True, NULL );
        }

        // Fix bug 3584 by not throwing an exception if we can't open a URL
        sal_Bool bOpened = ShellExec_openURL( aURL );

        if ( pSecurityScopedURL && pApplication_releaseSecurityScopedURL )
            pApplication_releaseSecurityScopedURL( pSecurityScopedURL );
#else	// USE_JAVA && MACOSX
#ifdef MACOSX
        aBuffer.append("open");
#else
        // The url launchers are expected to be in the $OOO_BASE_DIR/program
        // directory:
        com::sun::star::uno::Reference< com::sun::star::util::XMacroExpander >
            exp;
        if (!(m_xContext->getValueByName(
                  rtl::OUString(
                      RTL_CONSTASCII_USTRINGPARAM(
                          "/singletons/com.sun.star.util.theMacroExpander")))
              >>= exp)
            || !exp.is())
        {
            throw SystemShellExecuteException(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM(
                        "component context fails to supply singleton"
                        " com.sun.star.util.theMacroExpander of type"
                        " com.sun.star.util.XMacroExpander")),
                static_cast< XSystemShellExecute * >(this), ENOENT);
        }
        OUString aProgramURL;
        try {
            aProgramURL = exp->expandMacros(
                rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM("$OOO_BASE_DIR/program/")));
        } catch (com::sun::star::lang::IllegalArgumentException &)
        {
            throw SystemShellExecuteException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Could not expand $OOO_BASE_DIR path")), 
                static_cast < XSystemShellExecute * > (this), ENOENT );
        }
        
        OUString aProgram;
        if ( FileBase::E_None != FileBase::getSystemPathFromFileURL(aProgramURL, aProgram))
        {
            throw SystemShellExecuteException(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Cound not convert executable path")), 
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
        if ( m_aDesktopEnvironment.getLength() > 0 )
        {
            OString aDesktopEnvironment(m_aDesktopEnvironment.toAsciiLowerCase());
            OStringBuffer aCopy(aTmp);
            
            aCopy.append(aDesktopEnvironment);
            aCopy.append("-open-url");
            
            if ( 0 == access( aCopy.getStr(), X_OK) )
            {
                aBuffer.append(aDesktopEnvironment);
                aBuffer.append("-");

                /* CDE requires file urls to be decoded */                
                if ( m_aDesktopEnvironment.equals("CDE") && 0 == aURL.compareToAscii("file://", 7) )
                {
                    aURL = rtl::Uri::decode(aURL, rtl_UriDecodeWithCharset, osl_getThreadTextEncoding());
                }
            }
        }
             
        aBuffer.append("open-url");
#endif
        aBuffer.append(" ");
        escapeForShell(aBuffer, OUStringToOString(aURL, osl_getThreadTextEncoding()));
        
        if ( pDesktopLaunch && *pDesktopLaunch )
        {
            aLaunchBuffer.append( pDesktopLaunch );
            aLaunchBuffer.append(" ");
            escapeForShell(aLaunchBuffer, OUStringToOString(aURL, osl_getThreadTextEncoding()));
        }
#endif	// USE_JAVA && MACOSX
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
    if ( aLaunchBuffer.getLength() > 0 )
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

    OString cmd = aBuffer.makeStringAndClear();
    if ( 0 != pclose(popen(cmd.getStr(), "w")) )
    {
        int nerr = errno;
        throw SystemShellExecuteException(OUString::createFromAscii( strerror( nerr ) ), 
            static_cast < XSystemShellExecute * > (this), nerr );
    }
}


// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

OUString SAL_CALL ShellExec::getImplementationName(  ) 
    throw( RuntimeException )
{
	return OUString::createFromAscii( SHELLEXEC_IMPL_NAME );
}

// -------------------------------------------------
//	XServiceInfo
// -------------------------------------------------

sal_Bool SAL_CALL ShellExec::supportsService( const OUString& ServiceName ) 
    throw( RuntimeException )
{
    Sequence < OUString > SupportedServicesNames = ShellExec_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

// -------------------------------------------------
//	XServiceInfo
// -------------------------------------------------

Sequence< OUString > SAL_CALL ShellExec::getSupportedServiceNames(	 ) 
    throw( RuntimeException )
{
    return ShellExec_getSupportedServiceNames();
}

