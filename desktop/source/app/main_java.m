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
#include <unistd.h>
#include "osl/module.h"

#import <Cocoa/Cocoa.h>

#define MIN_MACOSX_MAJOR_VERSION 0x4
#define MAX_MACOSX_MAJOR_VERSION 0x6
#define TMPDIR "/tmp"

typedef OSErr Gestalt_Type( OSType selector, long *response );
typedef int SofficeMain_Type( int argc, char **argv );

static BOOL IsSupportedMacOSXVersion()
{
	BOOL bRet = NO;

	void *pLib = dlopen( NULL, RTLD_LAZY | RTLD_LOCAL );
	if ( pLib )
	{
		Gestalt_Type *pGestalt = (Gestalt_Type *)dlsym( pLib, "Gestalt" );
		if ( pGestalt )
		{
			// Currently we only support Mac OS X 10.4.x through 10.6.x
			long res = 0;
			pGestalt( gestaltSystemVersion, &res );
			bRet = ( ( ( ( res >> 8 ) & 0x00FF ) == 0x10 ) && ( ( ( res >> 4 ) & 0x000F ) >= MIN_MACOSX_MAJOR_VERSION ) && ( ( ( res >> 4 ) & 0x000F ) <= MAX_MACOSX_MAJOR_VERSION ) );
		}

		dlclose( pLib );
	}

	return bRet;
}

static NSString *GetNSTemporaryDirectory( const char *pProgName )
{
	NSString *pTempDir = nil;

	NSFileManager *pFileManager = [NSFileManager defaultManager];
	if ( pFileManager )
	{
		// Use NSCachesDirectory to stay within FileVault encryption
		NSArray *pCachePaths = NSSearchPathForDirectoriesInDomains( NSCachesDirectory, NSUserDomainMask, YES );
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
				if ( ( [pFileManager fileExistsAtPath:pCachePath isDirectory:&bDir] && bDir ) || [pFileManager createDirectoryAtPath:pCachePath attributes:pDict] )
				{
					// Append program name to cache path
					if ( pProgName )
					{
						pCachePath = [pCachePath stringByAppendingPathComponent:[NSString stringWithUTF8String:pProgName]];
						bDir = NO;
						if ( ( [pFileManager fileExistsAtPath:pCachePath isDirectory:&bDir] && bDir ) || [pFileManager createDirectoryAtPath:pCachePath attributes:pDict] )
						{
							pTempDir = pCachePath;
							break;
						}
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
	// Don't allow running as root as we really cannot trust that we won't
	// do any accidental damage
	if ( getuid() == 0 )
	{
		fprintf( stderr, "%s: running as root user is not allowed\n", argv[ 0 ] );
		_exit( 1 );
	}

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSBundle *pMainBundle = [NSBundle mainBundle];
	if ( !pMainBundle )
	{
		fprintf( stderr, "%s: application's main bundle is nil\n", argv[ 0 ] );
		[pPool release];
		_exit( 1 );
	}

	NSString *pBundlePath = [pMainBundle bundlePath];
	if ( !pBundlePath )
	{
		fprintf( stderr, "%s: application's main bundle path is nil\n", argv[ 0 ] );
		[pPool release];
		_exit( 1 );
	}

	NSString *pCmdPath = [pMainBundle executablePath];
	if ( !pCmdPath )
	{
		fprintf( stderr, "%s: application's executable path is nil\n", argv[ 0 ] );
		[pPool release];
		_exit( 1 );
	}

  	// Fix bug 3182 by detecting incorrectly formatted HOME values
  	const char *pEnvHome = getenv( "HOME" );
  	if ( pEnvHome )
  	{
  		// Make path absolute
		NSString *pHomePath = [NSString stringWithUTF8String:pEnvHome];
  		if ( *pEnvHome != '/' )
			pHomePath = [@"/" stringByAppendingString:pHomePath];
  		// Trim any trailing '/' characters
  		unsigned int nLen = [pHomePath length];
  		unsigned int i = nLen - 1;
  		while ( i && [pHomePath characterAtIndex:i] == '/' )
  			i--;
		if ( i < nLen - 1 )
			pHomePath = [pHomePath substringToIndex:i + 1];
		NSString *pHomeEnv = [NSString stringWithFormat:@"HOME=%@", pHomePath];
		putenv( (char *)[pHomeEnv UTF8String] );
  	}

	// Make sure TMPDIR exists as a softlink to /private/tmp as it can be
	// easily removed. In most cases, this call should fail, but we do it
	// just to be sure.
	symlink( "private/tmp", TMPDIR );

	// Fix bug 3631 by setting the temporary directory to something other
	// than /tmp if we can since Mac OS X will clear out the /tmp directory
	// periodically
	NSString *pTmpDir = GetNSTemporaryDirectory( argv[ 0 ] );
	NSString *pTmpEnv = [NSString stringWithFormat:@"TMPDIR=%@", pTmpDir];
	putenv( (char *)[pTmpEnv UTF8String] );
	pTmpEnv = [NSString stringWithFormat:@"TMP=%@", pTmpDir];
	putenv( (char *)[pTmpEnv UTF8String] );
	pTmpEnv = [NSString stringWithFormat:@"TEMP=%@", pTmpDir];
	putenv( (char *)[pTmpEnv UTF8String] );

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
		putenv( (char *)[pPathEnv UTF8String] );
	}

	// Fix bug 1198 and eliminate "libzip.jnilib not found" crashes by
	// unsetting DYLD_FRAMEWORK_PATH
	bool bRestart = false;
  	const char *pEnvFrameworkPath = getenv( "DYLD_FRAMEWORK_PATH" );
	// Always unset DYLD_FRAMEWORK_PATH
	unsetenv( "DYLD_FRAMEWORK_PATH" );
  	if ( pEnvFrameworkPath )
  	{
		NSString *pFrameworkPathEnv = [NSString stringWithFormat:@"DYLD_FALLBACK_FRAMEWORK_PATH=%@", [NSString stringWithUTF8String:pEnvFrameworkPath]];
  		const char *pEnvFallbackFrameworkPath = getenv( "DYLD_FALLBACK_FRAMEWORK_PATH" );
		if ( pEnvFallbackFrameworkPath )
			pFrameworkPathEnv = [pFrameworkPathEnv stringByAppendingFormat:@":%@", [NSString stringWithUTF8String:pEnvFallbackFrameworkPath]];
		putenv( (char *)[pFrameworkPathEnv UTF8String] );
		bRestart = true;
	}

	NSString *pStandardLibPath = [NSString stringWithFormat:@"%@/Contents/MacOS:%@/Contents/basis-link/program:%@/Contents/basis-link/ure-link/lib:/usr/lib:/usr/local/lib:", pBundlePath, pBundlePath, pBundlePath];
  	if ( pEnvHome )
		pStandardLibPath = [pStandardLibPath stringByAppendingFormat:@"%@/lib:", [NSString stringWithUTF8String:pEnvHome]];
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
		putenv( (char *)[pDyFallbackLibPathEnv UTF8String] );
		bRestart = true;
	}

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
				bRestart = true;
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
				bRestart = true;
			}

			CFRelease( aPref );
		}
	}

	// Restart if necessary since most library path changes don't have any
	// effect after the application has already started on most platforms
	if ( bRestart )
	{
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

		// Reexecute the parent process
		execv( [pCmdPath UTF8String], argv );
		fprintf( stderr, "%s: execv() function failed with error %i\n", argv[ 0 ], errno );
		[pPool release];
		_exit( 1 );
	}

	// If this Mac OS X version is not supported, try to open the bundled
	// "unsupported_macosx_version.html" file in the default web browser
	if ( !IsSupportedMacOSXVersion() )
	{
		NSString *pFile = @"unsupported_macosx_version";
		NSString *pExt = @"html";
		NSString *pHTMLPath = [pMainBundle pathForResource:pFile ofType:pExt];
		if ( !pHTMLPath )
		{
			pHTMLPath = [pMainBundle pathForResource:pFile ofType:pExt inDirectory:@"" forLocalization:@"en"];
			if ( !pHTMLPath )
				pHTMLPath = [NSString stringWithFormat:@"%@/Contents/Resources/en.lproj/%@.%@", pBundlePath, pFile, pExt];
		}

		if ( pHTMLPath )
		{
			NSWorkspace *pWorkspace = [NSWorkspace sharedWorkspace];
			if ( pWorkspace )
				[pWorkspace openFile:pHTMLPath];
		}
	}

	// File locking is enabled by default
	putenv( "SAL_ENABLE_FILE_LOCKING=1" );

	// Set Mono environment variables
	NSString *pTmpPath = [NSString stringWithFormat:@"MONO_ROOT=%@", pBundlePath];
	putenv( (char *)[pTmpPath UTF8String] );
	pTmpPath = [NSString stringWithFormat:@"MONO_CFG_DIR=%@", pBundlePath];
	putenv( (char *)[pTmpPath UTF8String] );
	pTmpPath = [NSString stringWithFormat:@"MONO_CONFIG=%@/mono/2.0/machine.config", pBundlePath];
	putenv( (char *)[pTmpPath UTF8String] );
	// Fix bug 2394 by turning off shared memory. Note that Mono is picky and
	// so the value must be set to "yes" to actually disable shared memory.
	putenv( "MONO_DISABLE_SHM=yes" );

	[pPool release];

	// Dynamically load soffice_main symbol to improve startup speed
	NSString *pSofficeLibPath = [NSString stringWithFormat:@"%@/Contents/basis-link/program/libsofficeapp.dylib", pBundlePath];
	void *pSofficeLib = dlopen( [pSofficeLibPath UTF8String], RTLD_LAZY | RTLD_LOCAL );
	if ( pSofficeLib )
	{
		SofficeMain_Type *pSofficeMain = (SofficeMain_Type *)dlsym( pSofficeLib, "soffice_main" );
		if ( pSofficeMain )
			return pSofficeMain( argc, argv );
	}

	return 0;
}
