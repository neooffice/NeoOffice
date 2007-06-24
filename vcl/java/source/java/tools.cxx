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
 *  Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
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

bool vcl::IsRunningPanther( )
{
	static bool initializedOnce = false;
	static bool isPanther = false;
	
	if ( ! initializedOnce )
	{
		long res = 0;
		Gestalt( gestaltSystemVersion, &res );
		isPanther = ( ( ( ( res >> 8 ) & 0x00FF ) == 0x10 ) && ( ( ( res >> 4 ) & 0x000F ) == 0x3 ) );
		initializedOnce = true;
	}
	
	return isPanther;
}

// ----------------------------------------------------------------------------

bool vcl::IsRunningTiger( )
{
	static bool initializedOnce = false;
	static bool isTiger = false;
	
	if ( ! initializedOnce )
	{
		long res = 0;
		Gestalt( gestaltSystemVersion, &res );
		isTiger = ( ( ( ( res >> 8 ) & 0x00FF ) == 0x10 ) && ( ( ( res >> 4 ) & 0x000F ) == 0x4 ) );
		initializedOnce = true;
	}
	
	return isTiger;
}

// ----------------------------------------------------------------------------

bool vcl::IsFullKeyboardAccessEnabled( )
{
	static bool initializedOnce = false;
	static bool isFullAccessEnabled = false;
	
	if ( ! initializedOnce )
	{
		CFPropertyListRef keyboardNavigationPref = NULL;
		keyboardNavigationPref = CFPreferencesCopyValue( CFSTR( "AppleKeyboardUIMode" ), kCFPreferencesAnyApplication, kCFPreferencesCurrentUser, kCFPreferencesAnyHost );
		if ( keyboardNavigationPref && CFGetTypeID( keyboardNavigationPref ) == CFNumberGetTypeID() && !CFNumberIsFloatType( (CFNumberRef)keyboardNavigationPref ) )
		{
			long prefVal;
			if ( CFNumberGetValue( (CFNumberRef)keyboardNavigationPref, kCFNumberLongType, &prefVal ) )
				isFullAccessEnabled = ( prefVal > 0 );
			CFRelease( keyboardNavigationPref );
		}
		initializedOnce = true;
	}
	
	return isFullAccessEnabled;
}
