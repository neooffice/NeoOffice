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
 */


#ifndef NO_LIBO_OPTIONAL_LOCALE_FIX
#include <sal/config.h>

#include <limits>
#endif	// !NO_LIBO_OPTIONAL_LOCALE_FIX

#include "localebackend.hxx"
#include <com/sun/star/beans/Optional.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <osl/time.h>
#ifndef NO_LIBO_OPTIONAL_LOCALE_FIX
#include <rtl/character.hxx>
#endif	// !NO_LIBO_OPTIONAL_LOCALE_FIX

#include <stdio.h>

#ifdef WNT
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#ifdef NO_LIBO_OPTIONAL_LOCALE_FIX
OUString ImplGetLocale(LCID lcid)
#else	// NO_LIBO_OPTIONAL_LOCALE_FIX
css::beans::Optional<css::uno::Any> ImplGetLocale(LCID lcid)
#endif	// NO_LIBO_OPTIONAL_LOCALE_FIX
{
    TCHAR buffer[8];
    LPTSTR cp = buffer;

    cp += GetLocaleInfo( lcid, LOCALE_SISO639LANGNAME , buffer, 4 );
    if( cp > buffer )
    {
        if( 0 < GetLocaleInfo( lcid, LOCALE_SISO3166CTRYNAME, cp, buffer + 8 - cp) )
            // #i50822# minus character must be written before cp
            *(cp - 1) = '-';

#ifdef NO_LIBO_OPTIONAL_LOCALE_FIX
        return OUString::createFromAscii(buffer);
#else	// NO_LIBO_OPTIONAL_LOCALE_FIX
        return {true, css::uno::Any(OUString::createFromAscii(buffer))};
#endif	// NO_LIBO_OPTIONAL_LOCALE_FIX
    }

#ifdef NO_LIBO_OPTIONAL_LOCALE_FIX
    return OUString();
#else	// NO_LIBO_OPTIONAL_LOCALE_FIX
    return {false, {}};
#endif	// NO_LIBO_OPTIONAL_LOCALE_FIX
}

#elif defined(MACOSX)

#include <rtl/ustrbuf.hxx>
#include <locale.h>
#include <string.h>

#include <premac.h>
#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>

namespace /* private */
{

    void OUStringBufferAppendCFString(OUStringBuffer& buffer, const CFStringRef s)
    {
        CFIndex lstr = CFStringGetLength(s);
        for (CFIndex i = 0; i < lstr; i++)
            buffer.append(CFStringGetCharacterAtIndex(s, i));
    }

    template <typename T>
    class CFGuard
    {
    public:
        explicit CFGuard(T& rT) : rT_(rT) {}
        ~CFGuard() { if (rT_) CFRelease(rT_); }
    private:
        T& rT_;
    };

    typedef CFGuard<CFArrayRef> CFArrayGuard;
    typedef CFGuard<CFStringRef> CFStringGuard;
    typedef CFGuard<CFTypeRef> CFTypeRefGuard;

    /* For more information on the Apple locale concept please refer to
    http://developer.apple.com/documentation/CoreFoundation/Conceptual/CFLocales/Articles/CFLocaleConcepts.html
    According to this documentation a locale identifier has the format: language[_country][_variant]*
    e.g. es_ES_PREEURO -> spain prior Euro support
    Note: The calling code should be able to handle locales with only language information e.g. 'en' for certain
    UI languages just the language code will be returned.
    */

    CFStringRef ImplGetAppPreference(const char* pref)
    {
        CFStringRef csPref = CFStringCreateWithCString(NULL, pref, kCFStringEncodingASCII);
        CFStringGuard csRefGuard(csPref);

        CFTypeRef ref = CFPreferencesCopyAppValue(csPref, kCFPreferencesCurrentApplication);
        CFTypeRefGuard refGuard(ref);

        if (ref == NULL)
            return NULL;

        CFStringRef sref = (CFGetTypeID(ref) == CFArrayGetTypeID()) ? (CFStringRef)CFArrayGetValueAtIndex((CFArrayRef)ref, 0) : (CFStringRef)ref;

        // NOTE: this API is only available with Mac OS X >=10.3. We need to use it because
        // Apple used non-ISO values on systems <10.2 like "German" for instance but didn't
        // upgrade those values during upgrade to newer Mac OS X versions. See also #i54337#
        return CFLocaleCreateCanonicalLocaleIdentifierFromString(kCFAllocatorDefault, sref);
    }

#ifdef NO_LIBO_OPTIONAL_LOCALE_FIX
    OUString ImplGetLocale(const char* pref)
#else	// NO_LIBO_OPTIONAL_LOCALE_FIX
    css::beans::Optional<css::uno::Any> ImplGetLocale(const char* pref)
#endif	// NO_LIBO_OPTIONAL_LOCALE_FIX
    {
        CFStringRef sref = ImplGetAppPreference(pref);
        CFStringGuard srefGuard(sref);

        OUStringBuffer aLocaleBuffer;
        aLocaleBuffer.appendAscii("en-US"); // initialize with fallback value

        if (sref != NULL)
        {
            // split the string into substrings; the first two (if there are two) substrings
            // are language and country
            CFArrayRef subs = CFStringCreateArrayBySeparatingStrings(NULL, sref, CFSTR("_"));
#ifdef USE_JAVA
            if (subs && CFArrayGetCount(subs) < 2)
            {
                CFRelease(subs);
 
                // Mac OS X will sometimes use "-" as its delimiter
                subs = CFStringCreateArrayBySeparatingStrings(NULL, sref, CFSTR("-"));
            }
#endif	// USE_JAVA
            CFArrayGuard subsGuard(subs);

            if (subs != NULL)
            {
                aLocaleBuffer.setLength(0); // clear buffer which still contains fallback value

                CFStringRef lang = (CFStringRef)CFArrayGetValueAtIndex(subs, 0);
#ifdef USE_JAVA
                if (CFStringCompare(lang, CFSTR("nn"), 0) == kCFCompareEqualTo || CFStringCompare(lang, CFSTR("no"), 0) == kCFCompareEqualTo)
                    lang = CFSTR("nb");
#endif	// USE_JAVA
                OUStringBufferAppendCFString(aLocaleBuffer, lang);

                // country also available? Assumption: if the array contains more than one
                // value the second value is always the country!
#ifdef USE_JAVA
                if (CFStringCompare(lang, CFSTR("pt"), 0) == kCFCompareEqualTo)
                {
                    aLocaleBuffer.appendAscii("-");
                    OUStringBufferAppendCFString(aLocaleBuffer, CFSTR("BR"));
                }
                else if (CFArrayGetCount(subs) > 1) 
#else	// USE_JAVA
                if (CFArrayGetCount(subs) > 1)
#endif	// USE_JAVA
                {
#ifdef USE_JAVA
                    CFStringRef country = (CFStringRef)CFArrayGetValueAtIndex(subs, 1);
                    if (CFStringGetLength(country) > 2)
                    {
                        if (CFStringCompare(country, CFSTR("Hans"), 0) == kCFCompareEqualTo)
                            country = CFSTR("CN");
                        else if (CFStringCompare(country, CFSTR("Hant"), 0) == kCFCompareEqualTo) 
                            country = CFSTR("TW");
                        else
                            country = NULL;
                    }
 
                    if (country)
                    {
                        aLocaleBuffer.appendAscii("-");
                        OUStringBufferAppendCFString(aLocaleBuffer, country);
                    }
#else	// USE_JAVA
                    aLocaleBuffer.appendAscii("-");
                    CFStringRef country = (CFStringRef)CFArrayGetValueAtIndex(subs, 1);
                    OUStringBufferAppendCFString(aLocaleBuffer, country);
#endif	// USE_JAVA
                }
            }
        }
#ifdef NO_LIBO_OPTIONAL_LOCALE_FIX
        return aLocaleBuffer.makeStringAndClear();
#else	// NO_LIBO_OPTIONAL_LOCALE_FIX
        return {true, css::uno::Any(aLocaleBuffer.makeStringAndClear())};
#endif	// NO_LIBO_OPTIONAL_LOCALE_FIX
    }

} // namespace /* private */

#else

#include <rtl/ustrbuf.hxx>
#include <locale.h>
#include <string.h>

/*
 * Note: setlocale is not at all thread safe, so is this code. It could
 * especially interfere with the stuff VCL is doing, so make sure this
 * is called from the main thread only.
 */

#ifdef NO_LIBO_OPTIONAL_LOCALE_FIX
static OUString ImplGetLocale(int category)
#else	// NO_LIBO_OPTIONAL_LOCALE_FIX
static css::beans::Optional<css::uno::Any> ImplGetLocale(char const * category)
#endif	// NO_LIBO_OPTIONAL_LOCALE_FIX
{
    const char *locale = setlocale(category, "");

    // Return "en-US" for C locales
    if( (locale == NULL) || ( locale[0] == 'C' && locale[1] == '\0' ) )
#ifdef NO_LIBO_OPTIONAL_LOCALE_FIX
        return OUString( "en-US"  );
#else	// NO_LIBO_OPTIONAL_LOCALE_FIX
        return {true, css::uno::Any(OUString("en-US"))};
#endif	// NO_LIBO_OPTIONAL_LOCALE_FIX


    const char *cp;
    const char *uscore = NULL;

    // locale string have the format lang[_ctry][.encoding][@modifier]
    // we are only interested in the first two items, so we handle
    // '.' and '@' as string end.
    for (cp = locale; *cp; cp++)
    {
        if (*cp == '_')
            uscore = cp;
        if (*cp == '.' || *cp == '@')
            break;
#ifndef NO_LIBO_OPTIONAL_LOCALE_FIX
        if (!rtl::isAscii(static_cast<unsigned char>(*cp))) {
            SAL_INFO("shell", "locale env var with non-ASCII content");
            return {false, {}};
        }
    }
    if (cp - locale > std::numeric_limits<sal_Int32>::max()) {
        SAL_INFO("shell", "locale env var content too long");
        return {false, {}};
#endif	// !NO_LIBO_OPTIONAL_LOCALE_FIX
    }

    OUStringBuffer aLocaleBuffer;
    if( uscore != NULL )
    {
        aLocaleBuffer.appendAscii(locale, uscore++ - locale);
        aLocaleBuffer.appendAscii("-");
        aLocaleBuffer.appendAscii(uscore, cp - uscore);
    }
    else
    {
        aLocaleBuffer.appendAscii(locale, cp - locale);
    }

#ifdef NO_LIBO_OPTIONAL_LOCALE_FIX
    return aLocaleBuffer.makeStringAndClear();
#else	// NO_LIBO_OPTIONAL_LOCALE_FIX
    return {true, css::uno::Any(aLocaleBuffer.makeStringAndClear())};
#endif	// NO_LIBO_OPTIONAL_LOCALE_FIX
}

#endif



LocaleBackend::LocaleBackend()
{
}



LocaleBackend::~LocaleBackend(void)
{
}



LocaleBackend* LocaleBackend::createInstance()
{
    return new LocaleBackend;
}



#ifdef NO_LIBO_OPTIONAL_LOCALE_FIX
OUString LocaleBackend::getLocale(void)
#else	// NO_LIBO_OPTIONAL_LOCALE_FIX
css::beans::Optional<css::uno::Any> LocaleBackend::getLocale()
#endif	// NO_LIBO_OPTIONAL_LOCALE_FIX
{
#if defined WNT
    return ImplGetLocale( GetUserDefaultLCID() );
#elif defined (MACOSX)
    return ImplGetLocale("AppleLocale");
#else
    return ImplGetLocale(LC_CTYPE);
#endif
}



#ifdef NO_LIBO_OPTIONAL_LOCALE_FIX
OUString LocaleBackend::getUILocale(void)
#else	// NO_LIBO_OPTIONAL_LOCALE_FIX
css::beans::Optional<css::uno::Any> LocaleBackend::getUILocale()
#endif	// NO_LIBO_OPTIONAL_LOCALE_FIX
{
#if defined WNT
    return ImplGetLocale( MAKELCID(GetUserDefaultUILanguage(), SORT_DEFAULT) );
#elif defined(MACOSX)
    return ImplGetLocale("AppleLanguages");
#else
    return ImplGetLocale(LC_MESSAGES);
#endif
}



#ifdef NO_LIBO_OPTIONAL_LOCALE_FIX
OUString LocaleBackend::getSystemLocale(void)
#else	// NO_LIBO_OPTIONAL_LOCALE_FIX
css::beans::Optional<css::uno::Any> LocaleBackend::getSystemLocale()
#endif	// NO_LIBO_OPTIONAL_LOCALE_FIX
{
// note: the implementation differs from getLocale() only on Windows
#if defined WNT
    return ImplGetLocale( GetSystemDefaultLCID() );
#else
    return getLocale();
#endif
}


void LocaleBackend::setPropertyValue(
    OUString const &, css::uno::Any const &)
    throw (
        css::beans::UnknownPropertyException, css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
        css::uno::RuntimeException, std::exception)
{
    throw css::lang::IllegalArgumentException(
        OUString(
            "setPropertyValue not supported"),
        static_cast< cppu::OWeakObject * >(this), -1);
}

css::uno::Any LocaleBackend::getPropertyValue(
    OUString const & PropertyName)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException, std::exception)
{
    if ( PropertyName == "Locale" ) {
#ifdef NO_LIBO_OPTIONAL_LOCALE_FIX
        return css::uno::makeAny(
            css::beans::Optional< css::uno::Any >(
                true, css::uno::makeAny(getLocale())));
#else	// NO_LIBO_OPTIONAL_LOCALE_FIX
        return css::uno::Any(getLocale());
#endif	// NO_LIBO_OPTIONAL_LOCALE_FIX
    } else if (PropertyName.equals("SystemLocale"))
    {
#ifdef NO_LIBO_OPTIONAL_LOCALE_FIX
        return css::uno::makeAny(
            css::beans::Optional< css::uno::Any >(
                true, css::uno::makeAny(getSystemLocale())));
#else	// NO_LIBO_OPTIONAL_LOCALE_FIX
        return css::uno::Any(getSystemLocale());
#endif	// NO_LIBO_OPTIONAL_LOCALE_FIX
    } else if (PropertyName.equals("UILocale"))
    {
#ifdef NO_LIBO_OPTIONAL_LOCALE_FIX
        return css::uno::makeAny(
            css::beans::Optional< css::uno::Any >(
                true, css::uno::makeAny(getUILocale())));
#else	// NO_LIBO_OPTIONAL_LOCALE_FIX
        return css::uno::Any(getUILocale());
#endif	// NO_LIBO_OPTIONAL_LOCALE_FIX
    } else {
        throw css::beans::UnknownPropertyException(
            PropertyName, static_cast< cppu::OWeakObject * >(this));
    }
}



OUString SAL_CALL LocaleBackend::getBackendName(void) {
    return OUString("com.sun.star.comp.configuration.backend.LocaleBackend") ;
}

OUString SAL_CALL LocaleBackend::getImplementationName(void)
    throw (uno::RuntimeException, std::exception)
{
    return getBackendName() ;
}

uno::Sequence<OUString> SAL_CALL LocaleBackend::getBackendServiceNames(void)
{
    uno::Sequence<OUString> aServiceNameList(1);
    aServiceNameList[0] = "com.sun.star.configuration.backend.LocaleBackend";
    return aServiceNameList ;
}

sal_Bool SAL_CALL LocaleBackend::supportsService(const OUString& aServiceName)
    throw (uno::RuntimeException, std::exception)
{
    return cppu::supportsService(this, aServiceName);
}

uno::Sequence<OUString> SAL_CALL LocaleBackend::getSupportedServiceNames(void)
    throw (uno::RuntimeException, std::exception)
{
    return getBackendServiceNames() ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
