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

#import <Cocoa/Cocoa.h>

#define TMPDIR "/var/tmp"

typedef int UnopkgMain_Type( int argc, char **argv );

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
	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	// Use CFBundle as [NSBundle mainBundle] will cause Java menu load failures
	CFBundleRef aMainBundle = CFBundleGetMainBundle();
	if ( !aMainBundle )
	{
		fprintf( stderr, "%s: application's main bundle is nil\n", argv[ 0 ] );
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
		fprintf( stderr, "%s: application's main bundle path is nil\n", argv[ 0 ] );
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
		fprintf( stderr, "%s: application's main bundle path missing program softlink\n", argv[ 0 ] );
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
		putenv( strdup( [pHomeEnv UTF8String] ) );
  	}

	// Fix bug 3631 by setting the temporary directory to something other
	// than /tmp if we can since Mac OS X will clear out the /tmp directory
	// periodically. Note that sources in sal/osl/unx will use these environment
	// variables as well.
	NSString *pTmpDir = GetNSTemporaryDirectory( argv[ 0 ] );
	NSString *pTmpEnv = [NSString stringWithFormat:@"TMPDIR=%@", pTmpDir];
	putenv( strdup( [pTmpEnv UTF8String] ) );
	pTmpEnv = [NSString stringWithFormat:@"TMP=%@", pTmpDir];
	putenv( strdup( [pTmpEnv UTF8String] ) );
	pTmpEnv = [NSString stringWithFormat:@"TEMP=%@", pTmpDir];
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

	// File locking is enabled by default
	putenv( "SAL_ENABLE_FILE_LOCKING=1" );

	// Dynamically load unopkg_main symbol to improve startup speed
	NSString *pUnopkgLibPath = [NSString stringWithFormat:@"%@/Contents/basis-link/program/libunopkgapp.dylib", pBundlePath];
	void *pUnopkgLib = dlopen( [pUnopkgLibPath UTF8String], RTLD_LAZY | RTLD_LOCAL );

	int nRet = 0;
	if ( pUnopkgLib )
	{
		UnopkgMain_Type *pUnopkgMain = (UnopkgMain_Type *)dlsym( pUnopkgLib, "unopkg_main" );
		if ( pUnopkgMain )
			nRet = pUnopkgMain( argc, argv );
	}

	[pPool release];

	return nRet;
}
