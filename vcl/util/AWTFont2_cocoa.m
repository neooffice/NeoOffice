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

#import <Cocoa/Cocoa.h>
#import <jni.h>
#import "AWTFont_cocoa.h"

// Fix for bug 1928. Java 1.5 and higher will try to set its own arbitrary
// italic angle so we create a custom implementation of the JVM's private
// AWTFont class to ignore the custom transform that Java passes in.
// Note: this file should only be loaded on Mac OS X 10.5 or higher.

static jmethodID mGetPSNameID = nil;

@interface NSFont (AWTFontRef)
- (ATSFontRef)_atsFontID;
- (id)initWithFontRef:(unsigned long)aATSFontRef size:(float)fSize;
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
		NSFont *pFont = nil;

		// In most cases, Java passes the display to this selector
		ATSFontRef aATSFont = ATSFontFindFromName( (CFStringRef)pName, kATSOptionFlagsDefault );
		if ( !aATSFont )
			aATSFont = ATSFontFindFromPostScriptName( (CFStringRef)pName, kATSOptionFlagsDefault );
		if ( aATSFont )
			pFont = [[NSFont alloc] initWithFontRef:(unsigned long)aATSFont size:(float)12];

		// Fallback if we could not find an NSFont using the ATS functions
		if ( !pFont )
			pFont = [NSFont fontWithName:pName size:(float)12];

		if ( pFont )
		{
			// Add to autorelease pool as invoking alloc disables
			// autorelease
			[pFont autorelease];

			pRet = [[AWTFont alloc] initWithFont:pFont isFakeItalic:bFakeItalic];
			if ( pRet )
			{
				// Add to autorelease pool as invoking alloc disables
				// autorelease
				[pRet autorelease];
			}
		}
	}

	return pRet;
}

+ (id)nsFontForJavaFont:(jobject)aFont env:(JNIEnv *)pEnv
{
	NSFont *pRet = nil;

	if ( aFont && pEnv )
	{
		// Fix bug 3031 by using retrieving the cached native font if it exists
		jclass aFontClass = (*pEnv)->GetObjectClass( pEnv, aFont );
		if ( aFontClass )
		{
			if ( !mGetPSNameID )
			{
				char *cSignature = "()Ljava/lang/String;";
				mGetPSNameID = (*pEnv)->GetMethodID( pEnv, aFontClass, "getPSName", cSignature );
			}
			if ( mGetPSNameID )
			{
				jstring tempObj = (jstring)( (*pEnv)->CallObjectMethod( pEnv, aFont, mGetPSNameID ) );
				if ( tempObj )
				{
					jboolean bCopy = JNI_FALSE;
					const char *pChars = (*pEnv)->GetStringUTFChars( pEnv, tempObj, &bCopy );
					if ( pChars )
					{
						NSString *pName = [NSString stringWithUTF8String:pChars];
						if ( pName )
						{
							ATSFontRef aATSFont = ATSFontFindFromPostScriptName( (CFStringRef)pName, kATSOptionFlagsDefault );
							if ( aATSFont )
								pRet = [[NSFont alloc] initWithFontRef:(unsigned long)aATSFont size:(float)12];

							// Fallback if we could not find an NSFont using
							// the ATS functions
							if ( !pRet )
								pRet = [NSFont fontWithName:pName size:(float)12];

							if ( pRet )
							{
								// Add to autorelease pool as invoking alloc
								// disables autorelease
								[pRet autorelease];
							}
						}
					}
				}
				else if ( (*pEnv)->ExceptionCheck( pEnv ) )
				{
					(*pEnv)->ExceptionDescribe( pEnv );
					(*pEnv)->ExceptionClear( pEnv );
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
