#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ApplicationServices/ApplicationServices.h>

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
