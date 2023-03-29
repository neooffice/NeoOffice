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

#include <sal/types.h>
#include <assert.h>

#include <premac.h>
#ifndef IOS
#ifndef USE_JAVA
#include <CoreServices/CoreServices.h>
#endif	// !USE_JAVA
#endif
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>

#include <rtl/ustrbuf.hxx>

#include <nlsupport.hxx>

namespace
{
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
    typedef CFGuard<CFPropertyListRef> CFPropertyListGuard;

    /** Get the current process locale from system
    */
    CFStringRef getProcessLocale()
    {
        CFPropertyListRef pref = CFPreferencesCopyAppValue(CFSTR("AppleLanguages"), kCFPreferencesCurrentApplication);
        CFPropertyListGuard proplGuard(pref);

        if (pref == NULL) // return fallback value 'en_US'
             return CFStringCreateWithCString(kCFAllocatorDefault, "en_US", kCFStringEncodingASCII);

        CFStringRef sref = (CFGetTypeID(pref) == CFArrayGetTypeID()) ? (CFStringRef)CFArrayGetValueAtIndex((CFArrayRef)pref, 0) : (CFStringRef)pref;

        return CFLocaleCreateCanonicalLocaleIdentifierFromString(kCFAllocatorDefault, sref);
    }

    void append(rtl::OUStringBuffer & buffer, CFStringRef string) {
        CFIndex n = CFStringGetLength(string);
        CFStringGetCharacters(
            string, CFRangeMake(0, n), buffer.appendUninitialized(n));
    }
}

/** Grab current locale from system.
*/
rtl::OUString macosx_getLocale()
{
    CFStringRef sref = getProcessLocale();
    CFStringGuard sGuard(sref);

    assert(sref != NULL && "osxlocale.cxx: getProcessLocale must return a non-NULL value");

    // split the string into substrings; the first two (if there are two) substrings
    // are language and country
    CFArrayRef subs = CFStringCreateArrayBySeparatingStrings(NULL, sref, CFSTR("-"));
#ifdef USE_JAVA
    if (CFArrayGetCount(subs) < 2)
    {
        CFRelease(subs);

        // Mac OS X will sometimes use "_" as its delimiter
        subs = CFStringCreateArrayBySeparatingStrings(NULL, sref, CFSTR("_"));
    }
#endif	// USE_JAVA
    CFArrayGuard arrGuard(subs);

    rtl::OUStringBuffer buf;
    append(buf, (CFStringRef)CFArrayGetValueAtIndex(subs, 0));

    // country also available? Assumption: if the array contains more than one
    // value the second value is always the country!
    if (CFArrayGetCount(subs) > 1)
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
            buf.append("_");
            append(buf, country);
        }
#else	// USE_JAVA
        buf.append("_");
        append(buf, (CFStringRef)CFArrayGetValueAtIndex(subs, 1));
#endif	// USE_JAVA
    }
    // Append 'UTF-8' to the locale because the Mac OS X file
    // system interface is UTF-8 based and sal tries to determine
    // the file system locale from the locale information
    buf.append(".UTF-8");
    return buf.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
