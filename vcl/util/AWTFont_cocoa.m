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
 *  Copyright 2007 by Patrick Luby (patrick.luby@planamesa.com)
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
#import "AWTFont_cocoa.h"

// Fix for bug 1928. Java 1.5 and higher will try to set its own arbitrary
// italic angle so we create a custom implementation of the JVM's private
// AWTFont class to ignore the custom transform that Java passes in.
// Note: this class should only be loaded dynamically as it should not be
// used on Mac OS X 10.5 or later.

@interface AWTFont : NSObject
{
	NSFont*				fFont;
	float*				fTransform;
	float				fPointSize;
	NSCharacterSet*		fCharacterSet;
	CGFontRef			fNativeCGFont;
}
+ (id)fontWithFont:(NSFont *)pFont matrix:(float *)pTransform;
- (id)initWithFont:(NSFont *)pFont matrix:(float *)pTransform;
- (void)dealloc;
@end

@implementation AWTFont

+ (id)fontWithFont:(NSFont *)pFont matrix:(float *)pTransform
{
	return [[AWTFont alloc] initWithFont:pFont matrix:pTransform];
}

- (id)initWithFont:(NSFont *)pFont matrix:(float *)pTransform
{
	[super init];

	fFont = pFont;
	if ( fFont )
	{
		[fFont retain];
		fPointSize = [fFont pointSize];
		fCharacterSet = [fFont coveredCharacterSet];
		if ( fCharacterSet )
			[fCharacterSet retain];

		// Fix bug 1990 by caching and reusing CGFontRefs
		if ( [fFont respondsToSelector:@selector(_atsFontID)] )
            fNativeCGFont = CreateCachedCGFont( (ATSFontRef)[fFont _atsFontID] );
	}

	fTransform = nil;

	return self;
}

- (void)dealloc
{
	if ( fFont )
		[fFont release];

	if ( fCharacterSet )
		[fCharacterSet release];

	CGFontRelease( fNativeCGFont );

	[super dealloc];
}

@end
