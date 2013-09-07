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

#import <Cocoa/Cocoa.h>

#define TMPDIR "/var/tmp"

typedef int SofficeMain_Type( int argc, char **argv );
typedef int UnoPkgMain_Type( int argc, char **argv );

static NSString *GetNSTemporaryDirectory()
{
	NSString *pTempDir = nil;

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
					pCachePath = [pCachePath stringByAppendingPathComponent:[NSString stringWithUTF8String:PRODUCT_DIR_NAME]];
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
	// Determine if we are running in unopkg mode
	BOOL bUnoPkg = ( argc >= 2 && !strcmp( "-unopkg", argv[ 1 ] ) ? YES : NO );

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	// Don't allow running as root as we really cannot trust that we won't
	// do any accidental damage
	if ( !bUnoPkg && getuid() == 0 )
	{
		NSLog( @"Running as root user is not allowed" );
		[pPool release];
		_exit( 1 );
	}

	// Use CFBundle as [NSBundle mainBundle] will cause Java menu load failures
	CFBundleRef aMainBundle = CFBundleGetMainBundle();
	if ( !aMainBundle )
	{
		NSLog( @"Application's main bundle is nil" );
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

	// Check if application's directory softlinks have been converted to
	// regular directories. If they have, the application has been moved or
	// copied to a file system that does not support softlinks.
	NSFileManager *pFileManager = [NSFileManager defaultManager];
	NSString *pProgramPath = [pBundlePath stringByAppendingPathComponent:@"Contents"];
	if ( pProgramPath )
		pProgramPath = [pProgramPath stringByAppendingPathComponent:@"program"];
	if ( !pFileManager || !pProgramPath || ![pFileManager destinationOfSymbolicLinkAtPath:pProgramPath error:nil] )
	{
		NSLog( @"Application's main bundle path missing program softlink" );
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

	// Put mozilla NSS files somewhere other than the default of "/"
	pTmpEnv = [NSString stringWithFormat:@"MOZILLA_CERTIFICATE_FOLDER=%@", pTmpDir];
	putenv( strdup( [pTmpEnv UTF8String] ) );

	// Unset the CLASSPATH environment variable
	unsetenv( "CLASSPATH" );

	// Assign command's directory to PATH environment variable
  	const char *pEnvPath = getenv( "PATH" );
	NSString *pPath = ( pEnvPath ? [NSString stringWithUTF8String:pEnvPath] : nil );
	NSString *pStandardPath = [NSString stringWithFormat:@"%@/Contents/MacOS:%@/Contents/basis-link/program:%@/Contents/basis-link/ure-link/bin:/bin:/sbin:/usr/bin:/usr/sbin:", pBundlePath, pBundlePath, pBundlePath];
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

	NSString *pStandardLibPath = [NSString stringWithFormat:@"%@/Contents/MacOS:%@/Contents/basis-link/program:%@/Contents/basis-link/ure-link/lib:/usr/lib:/usr/local/lib:", pBundlePath, pBundlePath, pBundlePath];
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

	if ( !bUnoPkg )
	{
		// Use default launch options if there are no application arguments
		if ( argc < 2 || ( argc == 2 && !strncmp( "-psn", argv[ 1 ], 4 ) ) )
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

		NSString *pPageinPath = [NSString stringWithFormat:@"%@/Contents/basis-link/program/pagein", pBundlePath];
		if ( !access( [pPageinPath UTF8String], R_OK | X_OK ) )
		{
			int nCurrentArg = 0;
			char *pPageinArgs[ argc + 3 ];
			pPageinArgs[ nCurrentArg++ ] = (char *)[pPageinPath UTF8String];
			NSString *pPageinSearchArg = [NSString stringWithFormat:@"-L%@/Contents/basis-link/program", pBundlePath];
			pPageinArgs[ nCurrentArg++ ] = (char *)[pPageinSearchArg UTF8String];
			int i = 1;
			for ( ; i < argc; i++ )
			{
				if ( !strcmp( "-calc", argv[ i ] ) )
					pPageinArgs[ nCurrentArg++ ] = "@pagein-calc";
				else if ( !strcmp( "-draw", argv[ i ] ) )
					pPageinArgs[ nCurrentArg++ ] = "@pagein-draw";
				else if ( !strcmp( "-impress", argv[ i ] ) )
					pPageinArgs[ nCurrentArg++ ] = "@pagein-impress";
				else if ( !strcmp( "-writer", argv[ i ] ) )
					pPageinArgs[ nCurrentArg++ ] = "@pagein-writer";
			}
			if ( nCurrentArg == 1 )
				pPageinArgs[ nCurrentArg++ ] = "@pagein-writer";
			pPageinArgs[ nCurrentArg++ ] = "@pagein-common";
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
	}

	// File locking is enabled by default
	putenv( "SAL_ENABLE_FILE_LOCKING=1" );

	// Elminate libxml2 sandbox file-deny-read of /etc by turning off searching
	// for XML catalogs
	putenv( "XML_CATALOG_FILES=" );

	// Dynamically load app's main symbol to improve startup speed
	NSString *pAppMainLibPath = nil;
	if ( bUnoPkg )
		pAppMainLibPath = [NSString stringWithFormat:@"%@/Contents/basis-link/program/libunopkgapp.dylib", pBundlePath];
	else
		pAppMainLibPath = [NSString stringWithFormat:@"%@/Contents/basis-link/program/libsofficeapp.dylib", pBundlePath];

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
