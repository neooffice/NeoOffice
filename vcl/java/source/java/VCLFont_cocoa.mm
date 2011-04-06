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
 *  Patrick Luby, April 2010
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2011 Planamesa Inc.
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

#include <map>
#include <jni.h>

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#import <Cocoa/Cocoa.h>
#import "VCLFont_cocoa.h"

static ::std::map< CTFontRef, CGFontRef > aFontMap;
static ::osl::Mutex aFontMutex;

using namespace osl;

// ============================================================================

static CGFontRef CreateCachedCGFont( CTFontRef aFont )
{
	CGFontRef aRet = NULL;

	MutexGuard aGuard( aFontMutex );

	::std::map< CTFontRef, CGFontRef >::iterator it = aFontMap.find( aFont );
	if ( it != aFontMap.end() )
	{
		aRet = it->second;
	}
	else
	{
		aRet = CTFontCopyGraphicsFont( aFont, NULL );
		if ( aRet )
		{
			CFRetain( aFont );
			aFontMap[ aFont ] = aRet;
		}
	}

	if ( aRet )
		CGFontRetain( aRet );

	return aRet;
}

// Fix for bug 1928. Java 1.5 and higher will try to set its own arbitrary
// italic angle so we create a custom implementation of the JVM's private
// VCLFont class to ignore the custom transform that Java passes in.
// Note: this file should only be loaded on Mac OS X 10.5 or higher.

static jmethodID mGetPSNameID = nil;

@implementation VCLFont

+ (id)awtFontForName:(NSString *)pName style:(int)nStyle isFakeItalic:(BOOL)bFakeItalic
{
	VCLFont *pRet = nil;

	if ( pName )
	{
		NSFont *pFont = [NSFont fontWithName:pName size:(float)12];
		if ( pFont )
		{
			pRet = [[VCLFont alloc] initWithFont:pFont isFakeItalic:bFakeItalic];
			[pRet autorelease];
		}
	}

	return pRet;
}

+ (id)nsFontForJavaFont:(jobject)aFont env:(JNIEnv *)pEnv
{
	NSFont *pRet = nil;

	if ( aFont && pEnv )
	{
		jclass aFontClass = pEnv->GetObjectClass( aFont );
        if ( aFontClass )
        {
			if ( !mGetPSNameID )
			{
				char *cSignature = "()Ljava/lang/String;";
				mGetPSNameID = pEnv->GetMethodID( aFontClass, "getPSName", cSignature );
			}
			if ( mGetPSNameID )
			{
				jstring tempObj = (jstring)( pEnv->CallObjectMethod( aFont, mGetPSNameID ) );
				if ( tempObj )
				{
					jboolean bCopy = JNI_FALSE;
					const char *pChars = pEnv->GetStringUTFChars( tempObj, &bCopy );
					if ( pChars )
					{
						NSString *pName = [NSString stringWithUTF8String:pChars];
						if ( pName )
							pRet = [NSFont fontWithName:pName size:(float)12];
					}
				}
				else if ( pEnv && pEnv->ExceptionCheck() )
				{
					pEnv->ExceptionDescribe();
					pEnv->ExceptionClear();
				}
			}
		}
	}

	return pRet;
}

- (id)initWithFont:(NSFont *)pFont isFakeItalic:(BOOL)bFakeItalic
{
	[super init];

	fFont = pFont;
	if ( fFont )
	{
		[fFont retain];

		// Fix bug 1990 by caching and reusing CGFontRefs
		fNativeCGFont = CreateCachedCGFont( (CTFontRef)fFont );
	}

	fIsFakeItalic = NO;

	return self;
}

- (void)dealloc
{
	if ( fFont )
	{
		MutexGuard aGuard( aFontMutex );

		::std::map< CTFontRef, CGFontRef >::iterator it = aFontMap.find( (CTFontRef)fFont );
		if ( it != aFontMap.end() )
		{
			CFRelease( it->first);
			CGFontRelease( it->second );
			aFontMap.erase( it );
		}

		[fFont release];
	}

	CGFontRelease( fNativeCGFont );

	[super dealloc];
}

- (void)finalize
{
	if ( fFont )
	{
		
		[fFont release];
	}

	CGFontRelease( fNativeCGFont );

	[super finalize];
}

@end
