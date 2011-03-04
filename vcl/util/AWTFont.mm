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
 *  Patrick Luby, June 2007
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2007 Planamesa Inc.
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

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#import <Cocoa/Cocoa.h>
#include <jni.h>

static ::std::map< ATSFontRef, CGFontRef > aATSFontMap;
static ::osl::Mutex aATSFontMutex;

using namespace osl;

// ============================================================================

CGFontRef CreateCachedCGFont( ATSFontRef aATSFont )
{
	CGFontRef aFont = NULL;

	MutexGuard aGuard( aATSFontMutex );

	::std::map< ATSFontRef, CGFontRef >::iterator it = aATSFontMap.find( aATSFont );
	if ( it != aATSFontMap.end() )
	{
		aFont = it->second;
	}
	else
	{
		aFont = CGFontCreateWithPlatformFont( (void *)&aATSFont );
		if ( aFont )
			aATSFontMap[ aATSFont ] = aFont;
	}

	if ( aFont )
		CGFontRetain( aFont );

	return aFont;
}

// Fix for bug 1928. Java 1.5 and higher will try to set its own arbitrary
// italic angle so we create a custom implementation of the JVM's private
// AWTFont class to ignore the custom transform that Java passes in.
// Note: this file should only be loaded on Mac OS X 10.5 or higher.

static jmethodID mGetPSNameID = nil;

@interface NSFont (AWTFontRef)
- (ATSFontRef)_atsFontID;
@end

@interface AWTFont : NSObject
{
	NSFont*				fFont;
	CGFontRef			fNativeCGFont;
	BOOL				fIsFakeItalic;
}
+ (id)awtFontForName:(NSString *)pName style:(int)nStyle isFakeItalic:(BOOL)bFakeItalic;
+ (id)nsFontForJavaFont:(jobject)aFont env:(JNIEnv *)pEnv;
- (id)initWithFont:(NSFont *)pFont isFakeItalic:(BOOL)bFakeItalic;
- (void)dealloc;
- (void)finalize;
@end

@implementation AWTFont

+ (id)awtFontForName:(NSString *)pName style:(int)nStyle isFakeItalic:(BOOL)bFakeItalic
{
	AWTFont *pRet = nil;

	if ( pName )
	{
		NSFont *pFont = [NSFont fontWithName:pName size:(float)12];
		if ( pFont )
		{
			pRet = [[AWTFont alloc] initWithFont:pFont isFakeItalic:bFakeItalic];
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
		if ( [fFont respondsToSelector:@selector(_atsFontID)] )
            fNativeCGFont = CreateCachedCGFont( [fFont _atsFontID] );
	}

	fIsFakeItalic = NO;

	return self;
}

- (void)dealloc
{
	if ( fFont )
		[fFont release];

	CGFontRelease( fNativeCGFont );

	[super dealloc];
}

- (void)finalize
{
	if ( fFont )
		[fFont release];

	CGFontRelease( fNativeCGFont );

	[super finalize];
}

@end
