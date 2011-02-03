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

#ifndef _SV_JAVA_TOOLS_HXX
#include <java/tools.hxx>
#endif					

#ifdef __cplusplus
#include <premac.h>
#endif
#include <Carbon/Carbon.h>
#ifdef __cplusplus
#include <postmac.h>
#endif

using namespace vcl;

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
	static bool initializedOnce = false;
	static bool isLeopard = false;
	
	if ( ! initializedOnce )
	{
		long res = 0;
		Gestalt( gestaltSystemVersion, &res );
		isLeopard = ( ( ( ( res >> 8 ) & 0x00FF ) == 0x10 ) && ( ( ( res >> 4 ) & 0x000F ) == 0x5 ) );
		initializedOnce = true;
	}
	
	return isLeopard;
}

// ----------------------------------------------------------------------------

bool vcl::IsRunningSnowLeopard( )
{
	static bool initializedOnce = false;
	static bool isSnowLeopard = false;
	
	if ( ! initializedOnce )
	{
		long res = 0;
		Gestalt( gestaltSystemVersion, &res );
		isSnowLeopard = ( ( ( ( res >> 8 ) & 0x00FF ) == 0x10 ) && ( ( ( res >> 4 ) & 0x000F ) == 0x6 ) );
		initializedOnce = true;
	}
	
	return isSnowLeopard;
}

// ----------------------------------------------------------------------------

bool vcl::IsRunningLion( )
{
	static bool initializedOnce = false;
	static bool isLion = false;
	
	if ( ! initializedOnce )
	{
		long res = 0;
		Gestalt( gestaltSystemVersion, &res );
		isLion = ( ( ( ( res >> 8 ) & 0x00FF ) == 0x10 ) && ( ( ( res >> 4 ) & 0x000F ) == 0x7 ) );
		initializedOnce = true;
	}
	
	return isLion;
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
