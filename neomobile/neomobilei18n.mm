/*************************************************************************
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *		 - GNU General Public License Version 2.1
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2008 by Planamesa Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *************************************************************************/

#include "neomobilei18n.hxx"
#include "premac.h"
#import <Cocoa/Cocoa.h>
#include <CoreFoundation/CoreFoundation.h>
#include "postmac.h"
#include <map>

/**
 * Translated strings for US English
 */
class us_enStrings : public std::map<std::string, std::string> {
public:
	us_enStrings() : std::map<std::string, std::string>() {
		// pairs are "key", "translation"
		
		insert(value_type("Cancel", "Cancel"));
		insert(value_type("Download canceled.", "Download canceled."));
		insert(value_type("Exporting file...", "Exporting file..."));
		insert(value_type("Uploading file...", "Uploading file..."));
		insert(value_type("Loading...", "Loading..."));
		insert(value_type("Downloading file ... ","Downloading file ... "));
		insert(value_type("Download failed!", "Download failed!"));
		insert(value_type("NeoOffice Mobile", "NeoOffice Mobile"));
	};
};

/**
 * Lookup a string and retrieve a translated string.  If no translation
 * is available, default to english.
 */
std::string GetLocalizedString(const char *src)
{
	std::string toReturn(src);
	
	std::string myLocale;
	
	CFArrayRef languages=(CFArrayRef)CFPreferencesCopyValue(CFSTR("AppleLanguages"), kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost);
	if(languages)
	{
		myLocale=CFStringGetCStringPtr((CFStringRef)CFArrayGetValueAtIndex(languages, 0), kCFStringEncodingMacRoman);\
		CFRelease(languages);
	}
	
	if(!myLocale.empty())
	{
		if(myLocale=="en")
		{
			us_enStrings trans;
			if(trans.find(src)!=trans.end())
				toReturn=trans[src];
		}
	}
	
	return(toReturn);
}
