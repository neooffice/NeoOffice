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
 *		 - GNU General Public License Version 2.1
 *
 *  Patrick Luby, November 2010
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2010 Planamesa Inc.
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
#include <stdio.h>
#include <string>

#import <Cocoa/Cocoa.h>

#include "main_java.h"

#define TMPDIR "/var/tmp"
#define UNOPKGARG "-unopkg"

typedef OSErr Gestalt_Type( OSType selector, SInt32 *response );
typedef int SofficeMain_Type( int argc, char **argv );
typedef int UnoPkgMain_Type( int argc, char **argv );

static BOOL IsSupportedMacOSXVersion()
{
	// Allow users to disable the Mac OS X version check by using the
	// following command:
	//   defaults write $(PRODUCT_DOMAIN).$(PRODUCT_DIR_NAME) DisableMacOSXVersionCheck -bool YES
	CFPropertyListRef aPref = CFPreferencesCopyAppValue( CFSTR( "DisableMacOSXVersionCheck" ), kCFPreferencesCurrentApplication );
	if ( aPref && CFGetTypeID( aPref ) == CFBooleanGetTypeID() && CFBooleanGetValue( (CFBooleanRef)aPref ) )
		return YES;

	SInt32 nMajorMinOSVersion = 0;
	SInt32 nMinorMinOSVersion = 0;
#ifdef PRODUCT_MIN_OSVERSION
	char *pMinOSVersion = strdup( PRODUCT_MIN_OSVERSION );
	if ( pMinOSVersion )
	{
		char *pToken = strsep( &pMinOSVersion, "." );
		if ( pToken )
		{
			nMajorMinOSVersion = (SInt32)strtol( pToken, NULL, 10 );
			pToken = strsep( &pMinOSVersion, "." );
			if ( pToken )
				nMinorMinOSVersion = (SInt32)strtol( pToken, NULL, 10 );
		}
		free( pMinOSVersion );
	}
#endif	// PRODUCT_MIN_OSVERSION

	SInt32 nMajorMaxOSVersion = 0;
	SInt32 nMinorMaxOSVersion = 0;
#ifdef PRODUCT_MAX_OSVERSION
	char *pMaxOSVersion = strdup( PRODUCT_MAX_OSVERSION );
	if ( pMaxOSVersion )
	{
		char *pToken = strsep( &pMaxOSVersion, "." );
		if ( pToken )
		{
			nMajorMaxOSVersion = (SInt32)strtol( pToken, NULL, 10 );
			pToken = strsep( &pMaxOSVersion, "." );
			if ( pToken )
				nMinorMaxOSVersion = (SInt32)strtol( pToken, NULL, 10 );
		}
		free( pMaxOSVersion );
	}
#else	// PRODUCT_MIN_OSVERSION
	nMajorMaxOSVersion = 0xffff;
	nMinorMaxOSVersion = 0xffff;
#endif	// PRODUCT_MIN_OSVERSION

	BOOL bRet = NO;

	void *pLib = dlopen( NULL, RTLD_LAZY | RTLD_LOCAL );
	if ( pLib )
	{
		Gestalt_Type *pGestalt = (Gestalt_Type *)dlsym( pLib, "Gestalt" );
		if ( pGestalt )
		{
			SInt32 majorVersion = 0;
			pGestalt( gestaltSystemVersionMajor, &majorVersion );
			if ( majorVersion >= nMajorMinOSVersion && majorVersion <= nMajorMaxOSVersion )
			{
				if ( majorVersion > nMajorMinOSVersion && majorVersion < nMajorMaxOSVersion )
				{
					bRet = YES;
				}
				else
				{
					SInt32 minorVersion = 0;
 					pGestalt( gestaltSystemVersionMinor, &minorVersion );
					if ( nMajorMinOSVersion == nMajorMaxOSVersion )
						bRet = ( minorVersion >= nMinorMinOSVersion && minorVersion <= nMinorMaxOSVersion );
					else if ( majorVersion == nMajorMinOSVersion )
						bRet = ( minorVersion >= nMinorMinOSVersion );
					else if ( majorVersion == nMajorMaxOSVersion )
						bRet = ( minorVersion <= nMinorMaxOSVersion );
				}
			}
		}

		dlclose( pLib );
	}

	return bRet;
}

static NSString *GetNSTemporaryDirectory()
{
	NSString *pTempDir = nil;

#ifdef PRODUCT_NAME
	NSFileManager *pFileManager = [NSFileManager defaultManager];
	if ( pFileManager )
	{
		// Use NSApplicationSupportDirectory to stay within FileVault encryption
		NSArray *pCachePaths = NSSearchPathForDirectoriesInDomains( NSApplicationSupportDirectory, NSUserDomainMask, YES );
		if ( pCachePaths )
		{
			NSNumber *pPerms = [NSNumber numberWithUnsignedLong:( S_IRUSR | S_IWUSR | S_IXUSR )];
			NSDictionary *pDict = ( pPerms ? [NSDictionary dictionaryWithObject:pPerms forKey:NSFilePosixPermissions] : nil );

 			unsigned int nCount = [pCachePaths count];
 			unsigned int i = 0;
			for ( ; i < nCount && !pTempDir; i++ )
			{
				BOOL bDir = NO;
				NSString *pCachePath = (NSString *)[pCachePaths objectAtIndex:i];
				if ( ( [pFileManager fileExistsAtPath:pCachePath isDirectory:&bDir] && bDir ) || [pFileManager createDirectoryAtPath:pCachePath withIntermediateDirectories:NO attributes:pDict error:nil] )
				{
					// Append program name to cache path
					pCachePath = [pCachePath stringByAppendingPathComponent:[NSString stringWithUTF8String:PRODUCT_NAME]];
					bDir = NO;
					if ( ( [pFileManager fileExistsAtPath:pCachePath isDirectory:&bDir] && bDir ) || [pFileManager createDirectoryAtPath:pCachePath withIntermediateDirectories:NO attributes:pDict error:nil] )
					{
						pTempDir = pCachePath;
						break;
					}
				}
			}
		}
	}
#endif	// PRODUCT_NAME

	if ( !pTempDir )
	{
		pTempDir = NSTemporaryDirectory();
		if ( !pTempDir )
			pTempDir = @TMPDIR;
	}

	return pTempDir;
}

int java_main( int argc, char **argv )
{
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	// Use CFBundle as [NSBundle mainBundle] will cause Java menu load failures
	CFBundleRef aMainBundle = CFBundleGetMainBundle();
	if ( !aMainBundle )
	{
		NSLog( @"Application's main bundle is nil" );
		[pPool release];
		_exit( 1 );
	}

	NSString *pCmdPath = nil;
	CFURLRef aCmdURL = CFBundleCopyExecutableURL( aMainBundle );
	if ( aCmdURL )
	{
		pCmdPath = (NSString *)CFURLCopyFileSystemPath( aCmdURL, kCFURLPOSIXPathStyle );
		if ( pCmdPath )
			[pCmdPath autorelease];
		CFRelease( aCmdURL );
	}
	if ( !pCmdPath )
	{
		NSLog( @"Application's executable path is nil" );
		[pPool release];
		_exit( 1 );
	}

	// Determine if we are running in unopkg mode
	NSString *pCmdName = [pCmdPath lastPathComponent];
	BOOL bUnoPkg = ( ( ( argc >= 2 && !strcmp( UNOPKGARG, argv[ 1 ] ) ) || [@"unopkg" isEqualToString:pCmdName] || [@"unopkg.bin" isEqualToString:pCmdName] ) ? YES : NO );

	// Don't allow running as root as we really cannot trust that we won't
	// do any accidental damage
	if ( !bUnoPkg && getuid() == 0 )
	{
		NSLog( @"Running as root user is not allowed" );
		[pPool release];
		_exit( 1 );
	}

	NSString *pBundlePath = nil;
	CFURLRef aBundleURL = CFBundleCopyBundleURL( aMainBundle );
	if ( aBundleURL )
	{
		pBundlePath = (NSString *)CFURLCopyFileSystemPath( aBundleURL, kCFURLPOSIXPathStyle );
		if ( pBundlePath )
			[pBundlePath autorelease];
		CFRelease( aBundleURL );
	}
	if ( !pBundlePath )
	{
		NSLog( @"Application's main bundle path is nil" );
		[pPool release];
		_exit( 1 );
	}

	// Check if application's softlinks have been converted to regular files.
	// If they have, the application has been moved or copied to a file system
	// that does not support softlinks.
	NSFileManager *pFileManager = [NSFileManager defaultManager];
	NSString *pSofficercPath = [pBundlePath stringByAppendingPathComponent:@"Contents"];
	if ( pSofficercPath )
	{
		pSofficercPath = [pSofficercPath stringByAppendingPathComponent:@"MacOS"];
		if ( pSofficercPath )
			pSofficercPath = [pSofficercPath stringByAppendingPathComponent:@"sofficerc"];
	}
	if ( !pFileManager || !pSofficercPath || ![pFileManager destinationOfSymbolicLinkAtPath:pSofficercPath error:nil] )
	{
		NSLog( @"Application's main bundle path missing \"MacOS/bootstraprc\" softlink" );
		[pPool release];
		_exit( 1 );
	}

  	// Fix bug 3182 by detecting incorrectly formatted HOME values
	NSString *pHomeDir = NSHomeDirectory();
	if ( pHomeDir )
	{
		NSURL *pURL = [NSURL fileURLWithPath:pHomeDir];
		if ( pURL )
		{
			pURL = [pURL URLByStandardizingPath];
			if ( pURL )
			{
				NSString *pHomeDir = [pURL path];
				if ( pHomeDir )
				{
					NSString *pHomeEnv = [NSString stringWithFormat:@"HOME=%@", pHomeDir];
					if ( pHomeEnv )
					{
						const char *pHomeEnvStr = [pHomeEnv UTF8String];
						if ( pHomeEnvStr )
							putenv( strdup( pHomeEnvStr ) );
					}
				}
			}
		}
	}

	// Fix bug 3631 by setting the temporary directory to something other
	// than /tmp if we can since Mac OS X will clear out the /tmp directory
	// periodically. Note that sources in sal/osl/unx will use these environment
	// variables as well.
	NSString *pTmpDir = GetNSTemporaryDirectory();
	NSString *pTmpEnv = [NSString stringWithFormat:@"TMPDIR=%@", pTmpDir];
	putenv( strdup( [pTmpEnv UTF8String] ) );
	pTmpEnv = [NSString stringWithFormat:@"TMP=%@", pTmpDir];
	putenv( strdup( [pTmpEnv UTF8String] ) );
	pTmpEnv = [NSString stringWithFormat:@"TEMP=%@", pTmpDir];
	putenv( strdup( [pTmpEnv UTF8String] ) );


	// NSTemporaryDirectory() may reset the above temporary directory
	// environmental variables so invoke NSTemporaryDirectory() now and set
	// a custom environmental variable to be used by the sal/osl/unx sources
	NSTemporaryDirectory();
	pTmpEnv = [NSString stringWithFormat:@"SAL_TMPDIR=%@", pTmpDir];
	putenv( strdup( [pTmpEnv UTF8String] ) );

	// Put mozilla NSS files somewhere other than the default of "/"
	pTmpEnv = [NSString stringWithFormat:@"MOZILLA_CERTIFICATE_FOLDER=%@", pTmpDir];
	putenv( strdup( [pTmpEnv UTF8String] ) );

	// Unset the CLASSPATH environment variable
	unsetenv( "CLASSPATH" );

	// Assign command's directory to PATH environment variable
  	const char *pEnvPath = getenv( "PATH" );
	NSString *pPath = ( pEnvPath ? [NSString stringWithUTF8String:pEnvPath] : nil );
	NSString *pStandardPath = [NSString stringWithFormat:@"%@/Contents/MacOS:%@/Contents/program:/bin:/sbin:/usr/bin:/usr/sbin:", pBundlePath, pBundlePath, pBundlePath];
	if ( !pPath || [pPath length] < [pStandardPath length] || [pPath compare:pStandardPath options:NSLiteralSearch range:NSMakeRange( 0, [pStandardPath length] )] != NSOrderedSame )
	{
		NSString *pPathEnv = [NSString stringWithFormat:@"PATH=%@", pStandardPath];
		if ( pPath )
			pPathEnv = [pPathEnv stringByAppendingFormat:@":%@", pPath];
		putenv( strdup( [pPathEnv UTF8String] ) );
	}

	// Fix bug 1198 and eliminate "libzip.jnilib not found" crashes by
	// unsetting DYLD_FRAMEWORK_PATH
  	const char *pEnvFrameworkPath = getenv( "DYLD_FRAMEWORK_PATH" );
	// Always unset DYLD_FRAMEWORK_PATH
	unsetenv( "DYLD_FRAMEWORK_PATH" );
  	if ( pEnvFrameworkPath )
  	{
		NSString *pFrameworkPathEnv = [NSString stringWithFormat:@"DYLD_FALLBACK_FRAMEWORK_PATH=%@", [NSString stringWithUTF8String:pEnvFrameworkPath]];
  		const char *pEnvFallbackFrameworkPath = getenv( "DYLD_FALLBACK_FRAMEWORK_PATH" );
		if ( pEnvFallbackFrameworkPath )
			pFrameworkPathEnv = [pFrameworkPathEnv stringByAppendingFormat:@":%@", [NSString stringWithUTF8String:pEnvFallbackFrameworkPath]];
		putenv( strdup( [pFrameworkPathEnv UTF8String] ) );
	}

	NSString *pStandardLibPath = [NSString stringWithFormat:@"%@/Contents/MacOS:%@/Contents/program:/usr/lib:/usr/local/lib:", pBundlePath, pBundlePath, pBundlePath];
	const char *pEnvLibPath = getenv( "LD_LIBRARY_PATH" );
	NSString *pLibPath = ( pEnvLibPath ? [NSString stringWithUTF8String:pEnvLibPath] : nil );
	const char *pEnvDyLibPath = getenv( "DYLD_LIBRARY_PATH" );
	NSString *pDyLibPath = ( pEnvDyLibPath ? [NSString stringWithUTF8String:pEnvDyLibPath] : nil );
	const char *pEnvDyFallbackLibPath = getenv( "DYLD_FALLBACK_LIBRARY_PATH" );
	NSString *pDyFallbackLibPath = ( pEnvDyFallbackLibPath ? [NSString stringWithUTF8String:pEnvDyFallbackLibPath] : nil );
	// Always unset LD_LIBRARY_PATH and DYLD_LIBRARY_PATH
	unsetenv( "LD_LIBRARY_PATH" );
	unsetenv( "DYLD_LIBRARY_PATH" );
	if ( !pDyFallbackLibPath || [pDyFallbackLibPath length] < [pStandardLibPath length] || [pDyFallbackLibPath compare:pStandardLibPath options:NSLiteralSearch range:NSMakeRange( 0, [pStandardLibPath length] )] != NSOrderedSame )
	{
		NSString *pDyFallbackLibPathEnv = [NSString stringWithFormat:@"DYLD_FALLBACK_LIBRARY_PATH=%@", pStandardLibPath];
		if ( pLibPath )
			pDyFallbackLibPathEnv = [pDyFallbackLibPathEnv stringByAppendingFormat:@":%@", pLibPath];
		if ( pDyLibPath )
			pDyFallbackLibPathEnv = [pDyFallbackLibPathEnv stringByAppendingFormat:@":%@", pDyLibPath];
		if ( pDyFallbackLibPath )
			pDyFallbackLibPathEnv = [pDyFallbackLibPathEnv stringByAppendingFormat:@":%@", pDyFallbackLibPath];
		putenv( strdup( [pDyFallbackLibPathEnv UTF8String] ) );
	}

	if ( bUnoPkg )
	{
		// Insert UNOPKGARG if missing
		if ( argc < 2 || strcmp( UNOPKGARG, argv[ 1 ] ) )
		{
			char **pNewArgv = (char **)malloc( sizeof( char** ) * ( argc + 2 ) );
			memcpy( pNewArgv + 1, argv, sizeof( char** ) * argc );
			pNewArgv[ 0 ] = strdup( argv[ 0 ] );
			pNewArgv[ 1 ] = strdup( UNOPKGARG );
			argc++;

			pNewArgv[ argc ] = NULL;
			argv = pNewArgv;
		}
	}
	else
	{
		// Insert module argument if missing
		const char *pNewArg = NULL;
		if ( [@"sbase" isEqualToString:pCmdName] )
			pNewArg = "-base";
		else if ( [@"scalc" isEqualToString:pCmdName] )
			pNewArg = "-calc";
		else if ( [@"sdraw" isEqualToString:pCmdName] )
			pNewArg = "-draw";
		else if ( [@"simpress" isEqualToString:pCmdName] )
			pNewArg = "-impress";
		else if ( [@"smath" isEqualToString:pCmdName] )
			pNewArg = "-math";
		else if ( [@"swriter" isEqualToString:pCmdName] )
			pNewArg = "-writer";

		if ( pNewArg )
		{
			char **pNewArgv = (char **)malloc( sizeof( char** ) * ( argc + 2 ) );
			memcpy( pNewArgv + 1, argv, sizeof( char** ) * argc );
			pNewArgv[ 0 ] = strdup( argv[ 0 ] );
			pNewArgv[ 1 ] = strdup( pNewArg );
			argc++;

			pNewArgv[ argc ] = NULL;
			argv = pNewArgv;
		}
		// Use default launch options if there are no application arguments
		else if ( argc < 2 || ( argc == 2 && !strncmp( "-psn", argv[ 1 ], 4 ) ) )
		{
			CFPropertyListRef aPref = CFPreferencesCopyAppValue( CFSTR( "DefaultLaunchOptions" ), kCFPreferencesCurrentApplication );
			if ( aPref )
			{
				if ( CFGetTypeID( aPref ) == CFStringGetTypeID() )
				{
					char **pNewArgv = (char **)malloc( sizeof( char** ) * ( argc + 2 ) );
					memcpy( pNewArgv, argv, sizeof( char** ) * argc );

					pNewArgv[ argc ] = (char *)[(NSString *)aPref UTF8String];
					if ( pNewArgv[ argc ] )
					{
						pNewArgv[ argc ] = strdup( pNewArgv[ argc ] );
						argc++;
					}

					pNewArgv[ argc ] = NULL;
					argv = pNewArgv;
				}
				else if ( CFGetTypeID( aPref ) == CFArrayGetTypeID() )
				{
					CFIndex nArrayLen = CFArrayGetCount( (CFArrayRef)aPref );
					char **pNewArgv = (char **)malloc( sizeof( char** ) * ( argc + nArrayLen + 1 ) );
					memcpy( pNewArgv, argv, sizeof( char** ) * argc );

					int i = 0;
					for ( ; i < nArrayLen; i++ )
					{
						CFStringRef aElement = (CFStringRef)CFArrayGetValueAtIndex( (CFArrayRef)aPref, i );
						if ( aElement )
						{
							pNewArgv[ argc ] = (char *)[(NSString *)aElement UTF8String];
							if ( pNewArgv[ argc ] )
							{
								pNewArgv[ argc ] = strdup( pNewArgv[ argc ] );
								argc++;
							}
						}
					}

					pNewArgv[ argc ] = NULL;
					argv = pNewArgv;
				}

				CFRelease( aPref );
			}
		}

		NSString *pPageinPath = [NSString stringWithFormat:@"%@/Contents/program/pagein", pBundlePath];
		if ( !access( [pPageinPath UTF8String], R_OK | X_OK ) )
		{
			int nCurrentArg = 0;
			char *pPageinArgs[ argc + 3 ];
			pPageinArgs[ nCurrentArg++ ] = (char *)[pPageinPath UTF8String];
			NSString *pPageinSearchArg = [NSString stringWithFormat:@"-L%@/Contents/program", pBundlePath];
			pPageinArgs[ nCurrentArg++ ] = (char *)[pPageinSearchArg UTF8String];
			int i = 1;
			for ( ; i < argc; i++ )
			{
				if ( !strcmp( "-calc", argv[ i ] ) )
					pPageinArgs[ nCurrentArg++ ] = strdup( "@pagein-calc" );
				else if ( !strcmp( "-draw", argv[ i ] ) )
					pPageinArgs[ nCurrentArg++ ] = strdup( "@pagein-draw" );
				else if ( !strcmp( "-impress", argv[ i ] ) )
					pPageinArgs[ nCurrentArg++ ] = strdup( "@pagein-impress" );
				else if ( !strcmp( "-writer", argv[ i ] ) )
					pPageinArgs[ nCurrentArg++ ] = strdup( "@pagein-writer" );
			}
			if ( nCurrentArg == 1 )
				pPageinArgs[ nCurrentArg++ ] = strdup( "@pagein-writer" );
			pPageinArgs[ nCurrentArg++ ] = strdup( "@pagein-common" );
			pPageinArgs[ nCurrentArg++ ] = NULL;

			// Execute the pagein command in child process
			pid_t pid = fork();
			if ( !pid )
			{
				close( 0 );
				execvp( [pPageinPath UTF8String], pPageinArgs );
				_exit( 1 );
			}
			else if ( pid > 0 )
			{
				// Invoke waitpid to prevent zombie processes
				int status;
				while ( waitpid( pid, &status, 0 ) > 0 && EINTR == errno )
					usleep( 10 );
			}
		}

		// If this Mac OS X version is not supported, try to open the bundled
		// "unsupported_macosx_version.html" file in the default web browser
		if ( Application_canUseJava() && !IsSupportedMacOSXVersion() )
		{
			CFStringRef aFile = CFSTR( "unsupported_macosx_version" );
			CFStringRef aType = CFSTR( "html" );
			CFURLRef aHTMLURL = CFBundleCopyResourceURL( aMainBundle, aFile, aType, CFSTR( "" ) );
			if ( !aHTMLURL )
				aHTMLURL = CFBundleCopyResourceURLForLocalization( aMainBundle, aFile, aType, CFSTR( "" ), CFSTR( "en" ) );

			NSString *pHTMLPath = nil;
			if ( aHTMLURL )
			{
				pHTMLPath = (NSString *)CFURLCopyFileSystemPath( aHTMLURL, kCFURLPOSIXPathStyle );
				if ( pHTMLPath )
					[pHTMLPath autorelease];
				CFRelease( aHTMLURL );
			}
			if ( !pHTMLPath )
				pHTMLPath = [NSString stringWithFormat:@"%@/Contents/Resources/en.lproj/%@.%@", pBundlePath, (NSString *)aFile, (NSString *)aType];
			if ( pHTMLPath )
			{
				NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
				if ( pWorkspace )
				{
					NSURL *pURL = [NSURL fileURLWithPath:pHTMLPath];
					if ( pURL )
						if ( ![pWorkspace openURL:pURL] )
							[pWorkspace openURLs:[NSArray arrayWithObject:pURL] withAppBundleIdentifier:@"com.apple.Safari" options:NSWorkspaceLaunchDefault additionalEventParamDescriptor:nil launchIdentifiers:nil];
				}
			}
		}
	}

	// File locking is enabled by default
	putenv( strdup( "SAL_ENABLE_FILE_LOCKING=1" ) );

	// Elminate libxml2 sandbox file-deny-read of /etc by turning off searching
	// for XML catalogs
	putenv( strdup( "XML_CATALOG_FILES=" ) );

	// Dynamically load app's main symbol to improve startup speed
	NSString *pAppMainLibPath = nil;
	if ( bUnoPkg )
		pAppMainLibPath = [NSString stringWithFormat:@"%@/Contents/program/libunopkgapp.dylib", pBundlePath];
	else
		pAppMainLibPath = [NSString stringWithFormat:@"%@/Contents/program/libsofficeapp.dylib", pBundlePath];

	void *pAppMainLib = NULL;
	if ( pAppMainLibPath )
		pAppMainLib = dlopen( [pAppMainLibPath UTF8String], RTLD_LAZY | RTLD_GLOBAL );

	int nRet = 0;
	if ( pAppMainLib )
	{
		if ( bUnoPkg )
		{
			UnoPkgMain_Type *pUnoPkgMain = (UnoPkgMain_Type *)dlsym( pAppMainLib, "unopkg_main" );
			if ( pUnoPkgMain )
				nRet = pUnoPkgMain( argc, argv );
		}
		else
		{
			SofficeMain_Type *pSofficeMain = (SofficeMain_Type *)dlsym( pAppMainLib, "soffice_main" );
			if ( pSofficeMain )
				nRet = pSofficeMain( argc, argv );
		}
	}

	// Don't release the pool until after app's main function returns otherwise
	// spellchecking and the Services menu will not work
	[pPool release];

	return nRet;
}
