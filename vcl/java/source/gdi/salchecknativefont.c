/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <CoreText/CoreText.h>

int main( int argc, char **argv )
{
	if ( argc < 2 || !strlen( argv[ 1 ] ) )
		exit( 1 );

	// Create an attributed string for an Oriya character. The crashing bug also occurs in many, but not all, characters in the U+0900 through U+0C7F Unicode range.
	UniChar indicChars[2];
	indicChars[0] = 0x0b1f;
	indicChars[1] = 0x0020;
	CFStringRef string = CFStringCreateWithCharacters( NULL, indicChars, 2 );
	CFAttributedStringRef attrString = CFAttributedStringCreate( NULL, string, NULL );
	CFMutableAttributedStringRef mutableAttrString = CFAttributedStringCreateMutableCopy( NULL, 0, attrString );

	// Set font
	CFStringRef fontName = CFStringCreateWithCString( NULL, argv[1], kCFStringEncodingUTF8 );

	CTFontRef font = CTFontCreateWithName( fontName, 13.0, NULL );
	CFAttributedStringSetAttribute( mutableAttrString, CFRangeMake( 0, CFAttributedStringGetLength( attrString ) ), kCTFontAttributeName, font );

	// Create typesetter instance. OS X 10.11, but not earlier versions of OS X, this should crash in the IndicClassTable class constructor.
	CTTypesetterRef typesetter = CTTypesetterCreateWithAttributedString( mutableAttrString );

	CFRelease( typesetter );
	CFRelease( font );
	CFRelease( fontName );
	CFRelease( mutableAttrString );
	CFRelease( attrString );
	CFRelease( string );

	exit( 0 );
}
