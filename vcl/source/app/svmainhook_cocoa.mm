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
 *  Patrick Luby, April 2012
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2012 Planamesa Inc.
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

#include <dlfcn.h>

#include <rtl/digest.h>
#include <vcl/unohelp.hxx>

#include <premac.h>
#import <objc/objc-runtime.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>
#undef check

#include "svmainhook_cocoa.h"
#include "../../java/source/java/VCLEventQueue_cocoa.h"

#define DOSTRING( x ) #x
#define STRING( x ) DOSTRING( x )

#define DOFUNCTION( x ) MacOSBOOL SAL_DLLPUBLIC_EXPORT _##x ()
#define FUNCTION( x ) DOFUNCTION( x )

typedef MacOSBOOL BundleCheck_Type();

using namespace rtl;

@interface NSBundle (VCLBundle)
- (MacOSBOOL)loadNibNamed:(NSString *)pNibName owner:(id)pOwner topLevelObjects:(NSArray **)pTopLevelObjects;
@end

void NSApplication_run()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSBundle *pBundle = [NSBundle mainBundle];

		MacOSBOOL bBundleOK = NO;
		OUString aLibName = ::vcl::unohelper::CreateLibraryName( "vcl", TRUE );
		if ( aLibName.getLength() )
		{
			void *pLib = dlopen( OUStringToOString( aLibName, osl_getThreadTextEncoding() ).getStr(), RTLD_LAZY | RTLD_LOCAL );
			if ( pLib )
			{
				BundleCheck_Type *pBundleCheck = (BundleCheck_Type *)dlsym( pLib, "_" STRING( PRODUCT_MD5 ) );
				if ( pBundleCheck )
					bBundleOK = pBundleCheck();

				dlclose( pLib );
			}
		}

		if ( !bBundleOK )
		{
			NSLog( @"Application's main bundle info dictionary is damaged" );
			[pPool release];
			_exit( 1 );
		}

		if ( pBundle && [pBundle respondsToSelector:@selector(loadNibNamed:owner:topLevelObjects:)] )
 			[pBundle loadNibNamed:@"MainMenu" owner:pApp topLevelObjects:nil];
		else if ( class_getClassMethod( [NSBundle class], @selector(loadNibNamed:owner:) ) )
			[NSBundle loadNibNamed:@"MainMenu" owner:pApp];
		VCLEventQueue_installVCLEventQueueClasses();
		if ( pBundle )
		{
			// Make sure our application is registered with launch services
			NSURL *pBundleURL = [pBundle bundleURL];
			if ( pBundleURL )
				LSRegisterURL( (CFURLRef)pBundleURL, false );
		}
		[pApp run];
	}

	[pPool release];
}

extern "C" FUNCTION( PRODUCT_MD5 )
{
	MacOSBOOL bRet = NO;

	// Check if the info dictionary has been changed
	NSBundle *pBundle = [NSBundle mainBundle];
	if ( pBundle )
	{
		NSDictionary *pInfoDict = [pBundle infoDictionary];
		if ( pInfoDict )
		{
			NSString *pBundleIdentifier = [pInfoDict objectForKey:@"CFBundleIdentifier"];
			NSString *pBundleName = [pInfoDict objectForKey:@"CFBundleName"];
			if ( pBundleIdentifier && pBundleName )
			{
				NSString *pKey = [pBundleIdentifier stringByAppendingFormat:@"_%@", pBundleName];
				const char *pKeyString = [pKey UTF8String];
				if ( pKeyString )
				{
					sal_uInt8 aBuf[ RTL_DIGEST_LENGTH_MD5 ];
					NSMutableString *pKeyMD5 = [NSMutableString stringWithCapacity:sizeof( aBuf )];
					if ( pKeyMD5 && rtl_digest_MD5( pKeyString, strlen( pKeyString ), aBuf, sizeof( aBuf ) ) == rtl_Digest_E_None )
					{
						for ( size_t i = 0; i < sizeof( aBuf ); i++ )
							[pKeyMD5 appendFormat:@"%02x", aBuf[ i ]];

						NSString *pProductMD5 = [NSString stringWithUTF8String:STRING( PRODUCT_MD5 )];
						if ( pProductMD5 && [pProductMD5 length] && [pProductMD5 caseInsensitiveCompare:pKeyMD5] == NSOrderedSame )
							bRet = YES;
					}
				}
			}
		}
	}

	return bRet;
}
