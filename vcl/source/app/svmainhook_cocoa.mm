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
#include <signal.h>

#include <rtl/digest.h>
#include <vcl/unohelp.hxx>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#import <CommonCrypto/CommonDigest.h>
#import <IOKit/IOKitLib.h>
#import <Security/SecAsn1Coder.h>
#import <Security/SecAsn1Templates.h>
#include <postmac.h>
#undef check

#include "svmainhook_cocoa.h"
#include "../../java/source/java/VCLEventQueue_cocoa.h"

#define DOFUNCTION( x ) BOOL SAL_DLLPUBLIC_EXPORT _##x ()
#define FUNCTION( x ) DOFUNCTION( x )

typedef BOOL BundleCheck_Type();
typedef sal_Bool Application_canSave_Type();
typedef sal_Bool Application_isRunningInSandbox_Type();

// The following are custom data types for Apple's App Store receipt payload
// ASN.1 format as documented in the following URL:
// https://developer.apple.com/library/mac/releasenotes/General/ValidateAppStoreReceipt/Chapters/ValidateLocally.html

typedef struct
{
	SecAsn1Item				type;
	SecAsn1Item				version;
	SecAsn1Item				value;
} AppReceiptAttribute;

typedef struct
{
	AppReceiptAttribute**	mpAttrs;
} AppReceiptAttributes;

static const int nDefaultExitCode = 173;
static Application_canSave_Type *pApplication_canSave = nullptr;
static Application_isRunningInSandbox_Type *pApplication_isRunningInSandbox = nullptr;

static const SecAsn1Template aAttributeTemplate[] = {
	{ SEC_ASN1_SEQUENCE, 0, nullptr, sizeof( AppReceiptAttribute ) },
	{ SEC_ASN1_INTEGER, offsetof( AppReceiptAttribute, type ), nullptr, 0 },
	{ SEC_ASN1_INTEGER, offsetof( AppReceiptAttribute, version ), nullptr, 0 },
	{ SEC_ASN1_OCTET_STRING, offsetof( AppReceiptAttribute, value ), nullptr, 0 },
	{ 0, 0, nullptr, 0 }
};

static const SecAsn1Template aAttributeSetTemplate[] = {
	{ SEC_ASN1_SET_OF, 0, aAttributeTemplate, sizeof( AppReceiptAttributes ) },
	{ 0, 0, nullptr, 0 }
};

static int ImplConvertAttributeToInt( SecAsn1Item *pItem )
{
	int nRet = 0;

	if ( pItem && pItem->Data && pItem->Length && pItem->Length <= sizeof( int ) )
		memcpy( &nRet, pItem->Data, pItem->Length );

	return nRet;
}

static CFDataRef ImplCreateMacAddress()
{
	CFDataRef aRet = nullptr;

	mach_port_t aMasterPort;
	if ( IOMasterPort( MACH_PORT_NULL, &aMasterPort ) == KERN_SUCCESS )
	{
		CFDictionaryRef aMatchingDict = IOBSDNameMatching( aMasterPort, 0, "en0" );
		if ( aMatchingDict )
		{
			// Don't release dictionary as it is released by the
			// IOServiceGetMatchingServices() function
			io_iterator_t aIterator;
			if ( IOServiceGetMatchingServices( aMasterPort, aMatchingDict, &aIterator ) == KERN_SUCCESS )
			{
				io_object_t aService;
				while ( ( aService = IOIteratorNext( aIterator ) ) )
				{
					io_object_t aParentService;
					if ( IORegistryEntryGetParentEntry( aService, kIOServicePlane, &aParentService ) == KERN_SUCCESS )
					{
						aRet = static_cast< CFDataRef >( IORegistryEntryCreateCFProperty( aParentService, CFSTR( "IOMACAddress" ), kCFAllocatorDefault, 0 ) );

						IOObjectRelease( aParentService );

						if ( aRet )
							break;
					}

					IOObjectRelease( aService );
				}

				IOObjectRelease( aIterator );
			}
		}
	}

	return aRet;
}

void ImplHandleAbort( int /* nSig */ )
{
    // Force exit since NSApplication won't shutdown when only exit() is invoked
    _exit( 0 );
}

#ifdef PRODUCT_CHECKSUM
extern "C" FUNCTION( PRODUCT_CHECKSUM )
{
	return YES;
}
#endif	// PRODUCT_CHECKSUM

#if defined PRODUCT_CHECKSUM2
extern "C" FUNCTION( PRODUCT_CHECKSUM2 )
{
	return YES;
}
#endif	// PRODUCT_CHECKSUM2

#if defined PRODUCT_CHECKSUM3
extern "C" FUNCTION( PRODUCT_CHECKSUM3 )
{
	return YES;
}
#endif	// PRODUCT_CHECKSUM3

void NSApplication_run()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSBundle *pBundle = [NSBundle mainBundle];
		if ( pBundle )
		{
			BOOL bBundleOK = NO;
			NSDictionary *pInfoDict = [pBundle infoDictionary];
			if ( pInfoDict )
			{
				NSString *pBundleName = [pInfoDict objectForKey:@"CFBundleName"];
				NSString *pBundleIdentifier = [pInfoDict objectForKey:@"CFBundleIdentifier"];
				if ( pBundleName && pBundleIdentifier && pBundleName )
				{
					NSString *pKey = [pBundleName stringByAppendingFormat:@"_%@", pBundleIdentifier];
					const char *pKeyString = [pKey UTF8String];
					if ( pKeyString )
					{
						sal_uInt8 aBuf[ RTL_DIGEST_LENGTH_MD5 ];
						NSMutableString *pKeyMD5 = [NSMutableString stringWithCapacity:sizeof( aBuf )];
						if ( pKeyMD5 && rtl_digest_MD5( pKeyString, strlen( pKeyString ), aBuf, sizeof( aBuf ) ) == rtl_Digest_E_None )
						{
							[pKeyMD5 appendString:@"_"];
							for ( size_t i = 0; i < sizeof( aBuf ); i++ )
								[pKeyMD5 appendFormat:@"%02x", aBuf[ i ]];

							const char *pKeyMD5String = [pKeyMD5 UTF8String];
							BundleCheck_Type *pBundleCheck = reinterpret_cast< BundleCheck_Type * >( dlsym( RTLD_SELF, pKeyMD5String ) );
							if ( pBundleCheck )
								bBundleOK = pBundleCheck();
						}
					}
				}
			}

			if ( !bBundleOK )
			{
				NSLog( @"Application's main bundle info dictionary is damaged" );
				[pPool release];
				_exit( 1 );
			}

 			[pBundle loadNibNamed:@"MainMenu" owner:pApp topLevelObjects:nil];

			VCLEventQueue_installVCLEventQueueClasses();

			// Make sure our application is registered with launch services
			NSURL *pBundleURL = [pBundle bundleURL];
			if ( pBundleURL )
				LSRegisterURL( static_cast< CFURLRef >( pBundleURL ), false );

			// Fix deadlock waiting for the application mutex when Oracle's Java
			// calls [NSObject performSelectorOnMainThread:withObject:waitUntilDone:]
			// by adding our custom run loop mode to the list of common modes
			CFRunLoopAddCommonMode( CFRunLoopGetMain(), CFSTR( "AWTRunLoopMode" ) );

			// Attempt to stop crashing due to uncaught Objective-C exceptions
			// in the main thread by running until the [NSApplication run]
			// selector exits cleanly
			bool bContinue = true;
			while ( bContinue )
			{
				try
				{
					[pApp run];
					bContinue = false;
				}
				catch ( ... )
				{
				}
			}
		}
	}

	[pPool release];
}

@interface GetExitCode: NSObject
{
	int						mnExitCode;
}
+ (id)create;
- (void)getExitCode:(id)pObject;
- (int)exitCode;
- (id)init;
@end

@implementation GetExitCode

+ (id)create
{
	GetExitCode *pRet = [[GetExitCode alloc] init];
	[pRet autorelease];
	return pRet;
}

- (void)getExitCode:(id)pObject
{
	(void)pObject;

	// Close any windows still showing so that all windows
	// get the appropriate window closing delegate calls
	NSApplication *pApp = [NSApplication sharedApplication];
	if ( pApp )
	{
		NSArray *pWindows = [pApp windows];
		if ( pWindows )
		{
			for ( NSWindow *pWindow in pWindows )
			{
				if ( pWindow )
					[pWindow orderOut:pWindow];
			}
		}
	}

	mnExitCode = nDefaultExitCode;

	NSBundle *pBundle = [NSBundle mainBundle];
	if ( pBundle )
	{
		// Fix spurious crashes in CMS* functions by trapping SIGABRT and
		// SIGSEGV signals
 		signal( SIGABRT, &ImplHandleAbort );
 		signal( SIGSEGV, &ImplHandleAbort );

		NSURL *pURL = [pBundle appStoreReceiptURL];
		if ( pURL && [pURL isKindOfClass:[NSURL class]] )
			pURL = [pURL filePathURL];
		if ( pURL )
			pURL = [pURL URLByStandardizingPath];
		if ( pURL && [pURL checkResourceIsReachableAndReturnError:nil] )
		{
			NSData *pData = [NSData dataWithContentsOfURL:pURL];
			if ( pData && pData.length && pData.bytes )
			{
				CMSDecoderRef aDecoder = nullptr;
				if ( CMSDecoderCreate( &aDecoder ) == errSecSuccess && aDecoder )
				{
					if ( CMSDecoderUpdateMessage( aDecoder, pData.bytes, pData.length ) == errSecSuccess && CMSDecoderFinalizeMessage( aDecoder ) == errSecSuccess )
					{
						BOOL bSigned = NO;
						size_t nSigners = 0;
						if ( CMSDecoderGetNumSigners( aDecoder, &nSigners ) == errSecSuccess && nSigners > 0 )
						{
							SecPolicyRef aPolicy = SecPolicyCreateBasicX509();
							if ( aPolicy )
							{
								for ( size_t i = 0; i < nSigners; i++ )
								{
									CMSSignerStatus nSignerStatus = kCMSSignerUnsigned;
									if ( CMSDecoderCopySignerStatus( aDecoder, i, aPolicy, TRUE, &nSignerStatus, nullptr, nullptr ) == errSecSuccess && nSignerStatus == kCMSSignerValid )
									{
										bSigned = YES;
										break;
									}
								}

								CFRelease( aPolicy );
							}
						}

						if ( bSigned )
						{
							NSString *pIdentifier = nil;
							NSData *pIdentifierData = nil;
							NSString *pVersion = nil;
							NSData *pOpaque = nil;
							NSData *pHash = nil;
							CFDataRef aContent = nullptr;
							if ( CMSDecoderCopyContent( aDecoder, &aContent ) == errSecSuccess && aContent )
							{
								const UInt8 *pContentBytes = CFDataGetBytePtr( aContent );
								CFIndex nContentLen = CFDataGetLength( aContent );
								SecAsn1CoderRef aAsn1Decoder = nullptr;
								if ( pContentBytes && nContentLen && SecAsn1CoderCreate( &aAsn1Decoder ) == errSecSuccess && aAsn1Decoder )
								{
									AppReceiptAttributes aPayload = { nullptr };
									if ( SecAsn1Decode( aAsn1Decoder, pContentBytes, nContentLen, aAttributeSetTemplate, &aPayload ) == errSecSuccess && aPayload.mpAttrs )
									{
										for ( AppReceiptAttribute **pAttrs = aPayload.mpAttrs; pAttrs && *pAttrs; pAttrs++ )
										{
											AppReceiptAttribute *pAttr = *pAttrs;
											if ( !pAttr->type.Length )
												continue;

											// Use type values as documented in the following URL to read payload:
											// https://developer.apple.com/library/mac/releasenotes/General/ValidateAppStoreReceipt/Chapters/ValidateLocally.html
											switch ( ImplConvertAttributeToInt( &pAttr->type ) )
											{
												case 2:
												{
													SecAsn1Item aValueItem;
													if ( SecAsn1Decode( aAsn1Decoder, pAttr->value.Data, pAttr->value.Length, kSecAsn1UTF8StringTemplate, &aValueItem ) == errSecSuccess )
													{
														pIdentifier = [[NSString alloc] initWithBytes:aValueItem.Data length:aValueItem.Length encoding:NSUTF8StringEncoding];
														if ( pIdentifier )
															[pIdentifier autorelease];
													}
													pIdentifierData = [NSData dataWithBytes:pAttr->value.Data length:pAttr->value.Length];
													break;
												}
												case 3:
												{
													SecAsn1Item aValueItem;
													if ( SecAsn1Decode( aAsn1Decoder, pAttr->value.Data, pAttr->value.Length, kSecAsn1UTF8StringTemplate, &aValueItem ) == errSecSuccess )
													{
														pVersion = [[NSString alloc] initWithBytes:aValueItem.Data length:aValueItem.Length encoding:NSUTF8StringEncoding];
														if ( pVersion )
															[pVersion autorelease];
													}
													break;
												}
												case 4:
												{
													pOpaque = [NSData dataWithBytes:pAttr->value.Data length:pAttr->value.Length];
													break;
												}
												case 5:
												{
													pHash = [NSData dataWithBytes:pAttr->value.Data length:pAttr->value.Length];
													break;
												}
											}
										}
									}

									SecAsn1CoderRelease( aAsn1Decoder );
								}

								CFRelease( aContent );
							}

							if ( pIdentifier && pIdentifierData && pIdentifierData.bytes && pIdentifierData.length && pVersion && pOpaque && pOpaque.length && pOpaque.bytes && pHash && pHash.length == CC_SHA1_DIGEST_LENGTH && pHash.bytes )
							{
								NSDictionary *pInfoDict = [pBundle infoDictionary];
								if ( pInfoDict )
								{
									NSString *pBundleIdentifier = [pInfoDict objectForKey:@"CFBundleIdentifier"];
									if ( pBundleIdentifier && [pBundleIdentifier isEqualToString:pIdentifier] )
									{
										CFDataRef aMacAddress = ImplCreateMacAddress();
										if ( aMacAddress )
										{
											const UInt8 *pMacAddressBytes = CFDataGetBytePtr( aMacAddress );
											CFIndex nMacAddressLen = CFDataGetLength( aMacAddress );
											unsigned char aDigest[ CC_SHA1_DIGEST_LENGTH ];
											if ( pMacAddressBytes && nMacAddressLen )
											{
												CC_SHA1_CTX aContext;
												CC_SHA1_Init( &aContext );
												CC_SHA1_Update( &aContext, pMacAddressBytes, nMacAddressLen );
												CC_SHA1_Update( &aContext, pOpaque.bytes, pOpaque.length );
												CC_SHA1_Update( &aContext, pIdentifierData.bytes, pIdentifierData.length );
												CC_SHA1_Final( aDigest, &aContext );
												if ( !memcmp( aDigest, pHash.bytes, CC_SHA1_DIGEST_LENGTH ) )
													mnExitCode = 0;
											}
										}
									}
								}
							}
						}
					}
					CFRelease( aDecoder );
				}
			}
		}
	}
}

- (int)exitCode
{
	return mnExitCode;
}

- (id)init
{
	[super init];

	mnExitCode = 0;

	return self;
}

@end

void NSApplication_terminate()
{
	int nRet = nDefaultExitCode;

	if ( !pApplication_canSave )
		pApplication_canSave = reinterpret_cast< Application_canSave_Type* >( dlsym( RTLD_MAIN_ONLY, "Application_canSave" ) );
	if ( !pApplication_isRunningInSandbox )
		pApplication_isRunningInSandbox = reinterpret_cast< Application_isRunningInSandbox_Type* >( dlsym( RTLD_MAIN_ONLY, "Application_isRunningInSandbox" ) );
	if ( ( pApplication_isRunningInSandbox && !pApplication_isRunningInSandbox() ) || ( pApplication_canSave && !pApplication_canSave() ) )
		nRet = 0;
	else
		nRet = Application_validateReceipt();

    // Force exit since NSApplication won't shutdown when only exit() is invoked
    _exit( nRet );
}

int Application_validateReceipt()
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	GetExitCode *pGetExitCode = [GetExitCode create];
	NSArray *pModes = [NSArray arrayWithObjects:NSDefaultRunLoopMode, NSEventTrackingRunLoopMode, NSModalPanelRunLoopMode, @"AWTRunLoopMode", nil];
	[pGetExitCode performSelectorOnMainThread:@selector(getExitCode:) withObject:pGetExitCode waitUntilDone:YES modes:pModes];
	int nRet = [pGetExitCode exitCode];

	[pPool release];

    return nRet;
}
