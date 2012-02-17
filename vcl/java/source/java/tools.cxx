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
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 Planamesa Inc.
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
 ************************************************************************/

#define _SV_JAVA_TOOLS_CXX

#include <dlfcn.h>

#ifndef _SV_JAVA_TOOLS_HXX
#include <java/tools.hxx>
#endif					

#include <premac.h>
#include <CoreServices/CoreServices.h>
#include <postmac.h>

typedef OSErr Gestalt_Type( OSType selector, long *response );

static bool isLeopard = false;
static bool isSnowLeopard = false;
static bool isLion = false;
static bool isMountainLion = false;

using namespace vcl;

// ============================================================================

static void InitializeMacOSXVersion()
{
	static bool nInitialized = false;

	if ( nInitialized )
		return;
	
	void *pLib = dlopen( NULL, RTLD_LAZY | RTLD_LOCAL );
	if ( pLib )
	{
		Gestalt_Type *pGestalt = (Gestalt_Type *)dlsym( pLib, "Gestalt" );
		if ( pGestalt )
		{
			SInt32 res = 0;
			pGestalt( gestaltSystemVersionMajor, &res );
			if ( res == 10 )
			{
				res = 0;
				pGestalt( gestaltSystemVersionMinor, &res );
				switch ( res )
				{
					case 5:
						isLeopard = true;
						break;
					case 6:
						isSnowLeopard = true;
						break;
					case 7:
						isLion = true;
						break;
					case 8:
						isMountainLion = true;
						break;
					default:
						break;
				}
			}
		}

		dlclose( pLib );
	}

	nInitialized = true;
}

// ============================================================================

jstring vcl::StringToJavaString( JNIEnv *pEnv, const ::rtl::OUString& _rTemp )
{
	jstring pStr = NULL;
	if ( pEnv )
	{
		pStr = pEnv->NewString( _rTemp.getStr(), _rTemp.getLength() );
		pEnv->ExceptionClear();
		OSL_ENSURE( pStr, "Could not create a jstring object!" );
	}
	return pStr;
}

// ----------------------------------------------------------------------------

::rtl::OUString vcl::JavaString2String( JNIEnv *pEnv, jstring _Str )
{
	::rtl::OUString aStr;
	if ( _Str )
	{
		jboolean bCopy( sal_True );
		const jchar *pChar = pEnv->GetStringChars( _Str, &bCopy );
		jsize len = pEnv->GetStringLength( _Str );
		aStr = ::rtl::OUString( pChar, len );
		pEnv->ReleaseStringChars( _Str, pChar );
	}
	return aStr;
}

// ----------------------------------------------------------------------------

bool vcl::IsRunningLeopard( )
{
	InitializeMacOSXVersion();
	return isLeopard;
}

// ----------------------------------------------------------------------------

bool vcl::IsRunningSnowLeopard( )
{
	InitializeMacOSXVersion();
	return isSnowLeopard;
}

// ----------------------------------------------------------------------------

bool vcl::IsRunningLion( )
{
	InitializeMacOSXVersion();
	return isLion;
}

// ----------------------------------------------------------------------------

bool vcl::IsRunningMountainLion( )
{
	InitializeMacOSXVersion();
	return isMountainLion;
}

// ----------------------------------------------------------------------------

bool vcl::IsFullKeyboardAccessEnabled( )
{
	bool isFullAccessEnabled = false;

	CFPropertyListRef keyboardNavigationPref = CFPreferencesCopyAppValue( CFSTR( "AppleKeyboardUIMode" ), kCFPreferencesCurrentApplication );
	if ( keyboardNavigationPref )
	{
		int prefVal;
		if ( CFGetTypeID( keyboardNavigationPref ) == CFNumberGetTypeID() && CFNumberGetValue( (CFNumberRef)keyboardNavigationPref, kCFNumberIntType, &prefVal ) )
			isFullAccessEnabled = ( prefVal % 2 ? true : false );
		CFRelease( keyboardNavigationPref );
	}

	return isFullAccessEnabled;
}
