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
 *         - GNU General Public License Version 2.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
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
 *  =================================================
 *  Modified June 2003 by Patrick Luby. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/

#include <sal/types.h>

#include <premac.h>
#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <postmac.h>

/*
 * macxp_getOSXLocale
 *
 * Grab current locale from system.
 */
int macxp_getOSXLocale( char *locale, sal_uInt32 bufferLen )
{
	LocaleRef lref;
	CFArrayRef aref;
	CFStringRef	sref;
     
	aref = (CFArrayRef)CFPreferencesCopyAppValue( CFSTR( "AppleLanguages" ), kCFPreferencesCurrentUser );
	if ( aref != NULL && ( sref = (CFStringRef)CFArrayGetValueAtIndex( aref, 0 ) ) != NULL )
	{
        size_t len = 0;

        CFStringGetCString( sref, locale, bufferLen, CFStringGetSystemEncoding() );
        /* Sometimes CFPref AppleLanguages gets corrupted so check for it */
        len = strlen( locale );
        if ( len < 2 || ( len >= 5 && locale[2] != '_' ) )
        {
            fprintf( stderr, "The value of CFPref AppleLanguages is corrupted! Please remove and readd your preferred language from the list of languages in the International control panel to fix this problem.\n" );
            strcpy( locale, "en" );
        }
    }
    else
    {
        fprintf( stderr, "Could not get value of CFPref AppleLanguages! Please reset your locale in the International control panel.\n" );
        strcpy( locale, "en" );
    }

	return( noErr );
}


/*
 * macxp_OSXConvertCFEncodingToIANACharSetName
 *
 * Convert a CoreFoundation text encoding to an IANA charset name.
 */
int macxp_OSXConvertCFEncodingToIANACharSetName( char *buffer, unsigned int bufferLen, CFStringEncoding cfEncoding )
{
	CFStringRef	sCFEncodingName;

	sCFEncodingName = CFStringConvertEncodingToIANACharSetName( cfEncoding );
	CFStringGetCString( sCFEncodingName, buffer, bufferLen, cfEncoding );

	return( noErr );
}

