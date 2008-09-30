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
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.gnu.org/licenses/gpl-3.0.txt>
 * for a copy of the GPLv3 License.
 *
 * Modified January 2006 by Patrick Luby. NeoOffice is distributed under
 * GPL only under modification term 2 of the LGPL.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include <sal/types.h>
#include <assert.h>

#include <premac.h>
#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>

namespace /* private */
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
		CFPropertyListRef pref = CFPreferencesCopyAppValue(CFSTR("AppleLocale"), kCFPreferencesCurrentApplication);
		CFPropertyListGuard proplGuard(pref);
		
		if (pref == NULL) // return fallback value 'en_US'
			 return CFStringCreateWithCString(kCFAllocatorDefault, "en_US", kCFStringEncodingASCII);
		
		CFStringRef sref = (CFGetTypeID(pref) == CFArrayGetTypeID()) ? (CFStringRef)CFArrayGetValueAtIndex((CFArrayRef)pref, 0) : (CFStringRef)pref;
		
		// NOTE: this API is only available with Mac OS X >=10.3. We need to use it because
		// Apple used non-ISO values on systems <10.2 like "German" for instance but didn't
		// upgrade those values during upgrade to newer Mac OS X versions. See also #i54337#
		return CFLocaleCreateCanonicalLocaleIdentifierFromString(kCFAllocatorDefault, sref);	
	}	
} // namespace private
	
/** Grab current locale from system.
*/
extern "C" {
int macosx_getLocale(char *locale, sal_uInt32 bufferLen)
{	
	CFStringRef sref = getProcessLocale();
	CFStringGuard sGuard(sref);
	
	assert(sref != NULL && "osxlocale.cxx: getProcessLocale must return a non-NULL value");
	
	// split the string into substrings; the first two (if there are two) substrings 
	// are language and country
	CFArrayRef subs = CFStringCreateArrayBySeparatingStrings(NULL, sref, CFSTR("_"));	
#ifdef USE_JAVA
	if (CFArrayGetCount(subs) < 2)
	{
		CFRelease(subs);

		// Mac OS X will sometimes use "-" as its delimiter
		subs = CFStringCreateArrayBySeparatingStrings(NULL, sref, CFSTR("-"));	
	}
#endif	// USE_JAVA
	CFArrayGuard arrGuard(subs);
		
	CFStringRef lang = (CFStringRef)CFArrayGetValueAtIndex(subs, 0);
	CFStringGetCString(lang, locale, bufferLen, kCFStringEncodingASCII);
		
	// country also available? Assumption: if the array contains more than one
	// value the second value is always the country!
	if (CFArrayGetCount(subs) > 1)
	{
#ifndef USE_JAVA
		strlcat(locale, "_", bufferLen - strlen(locale));
#endif	// !USE_JAVA
			
		CFStringRef country = (CFStringRef)CFArrayGetValueAtIndex(subs, 1);
#ifdef USE_JAVA
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
			strlcat(locale, "_", bufferLen - strlen(locale));
			CFStringGetCString(country, locale + strlen(locale), bufferLen - strlen(locale), kCFStringEncodingASCII);
		}
#else	// USE_JAVA
		CFStringGetCString(country, locale + strlen(locale), bufferLen - strlen(locale), kCFStringEncodingASCII);			
#endif	// USE_JAVA
	}	    
    // Append 'UTF-8' to the locale because the Mac OS X file
    // system interface is UTF-8 based and sal tries to determine
    // the file system locale from the locale information 
	strlcat(locale, ".UTF-8", bufferLen - strlen(locale));							
		
	return noErr;
}
}



/*
 * macxp_OSXConvertCFEncodingToIANACharSetName
 *
 * Convert a CoreFoundation text encoding to an IANA charset name.
 */
extern "C" int macxp_OSXConvertCFEncodingToIANACharSetName( char *buffer, unsigned int bufferLen, CFStringEncoding cfEncoding )
{
	CFStringRef	sCFEncodingName;

	sCFEncodingName = CFStringConvertEncodingToIANACharSetName( cfEncoding );
	CFStringGetCString( sCFEncodingName, buffer, bufferLen, cfEncoding );

	if ( sCFEncodingName )
	    CFRelease( sCFEncodingName );

	return( noErr );
}

