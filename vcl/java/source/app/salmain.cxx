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
 *  Patrick Luby, June 2003
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
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

#define _SV_SALMAIN_CXX

#include <stdio.h>
#include <unistd.h>

#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif
#ifndef _FSYS_HXX
#include <tools/fsys.hxx>
#endif

#ifdef MACOSX

#ifndef _SV_SALMAIN_COCOA_H
#include <salmain_cocoa.h>
#endif
#ifndef _SV_JAVA_LANG_CLASS_HXX 
#include <java/lang/Class.hxx>
#endif
#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif
#include <crt_externs.h>

class SVMainThread : public ::vos::OThread
{
protected:
	virtual void run() { SVMain(); _exit( 0 ); }
};

using namespace osl;
using namespace rtl;
using namespace vcl;
using namespace vos;

static Mutex aCarbonLock;
static java_lang_Object *pCarbonLockObject = NULL;

#endif

// ============================================================================

#ifdef MACOSX
static jint JNICALL Java_com_apple_mrj_macos_carbon_CarbonLock_acquire0( JNIEnv *pEnv, jobject object )
{
	return ( aCarbonLock.tryToAcquire() ? 0 : 1 );
}
#endif

// ----------------------------------------------------------------------------

#ifdef MACOSX
static jobject JNICALL Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance( JNIEnv *pEnv, jobject object )
{
	return pCarbonLockObject->getJavaObject();
}
#endif

// ----------------------------------------------------------------------------

#ifdef MACOSX
static void JNICALL Java_com_apple_mrj_macos_carbon_CarbonLock_init( JNIEnv *pEnv, jobject object )
{
}
#endif

// ----------------------------------------------------------------------------

#ifdef MACOSX
static jint JNICALL Java_com_apple_mrj_macos_carbon_CarbonLock_release0( JNIEnv *pEnv, jobject object )
{
	return ( aCarbonLock.release() ? 0 : 1 );
}
#endif

// ----------------------------------------------------------------------------

BEGIN_C

int main( int argc, char *argv[] )
{

	char *pCmdPath = argv[ 0 ];

#ifdef MACOSX
	// We need to use _NSGetEnviron() here to get the path of this executable
	// because argv[0] does not have any directory when the executable is
	// found in the user's PATH environment variable
	char **ppEnviron = NULL;
	if(_NSGetEnviron())
		ppEnviron = *_NSGetEnviron();

	// Get full executable path. We can't use __progname as that only holds
	// the name of the executable and not the path. The full executable path
	// is listed after the first NULL in *environ.
	if ( ppEnviron ) {
		char **ppTmp;
		ppTmp = ppEnviron;
		while ( *ppTmp++ )
			;
		pCmdPath = *ppTmp;
	}

	// Get absolute path of command's directory
	ByteString aCmdPath( pCmdPath );
	if ( aCmdPath.Len() )
	{
		DirEntry aCmdDirEntry( aCmdPath );
		aCmdDirEntry.ToAbs();
		aCmdPath = ByteString( aCmdDirEntry.GetPath().GetFull(), RTL_TEXTENCODING_UTF8 );
	}

	// Assign command's directory to PATH environment variable
	ByteString aPath( getenv( "PATH" ) );
	if ( aCmdPath.Len() )
	{
		ByteString aTmpPath( "PATH=" );
		aTmpPath += aCmdPath;
		if ( aPath.Len() )
		{
			aTmpPath += ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_UTF8 );
			aTmpPath += aPath;
		}
		putenv( aTmpPath.GetBuffer() );
	}

	// Assign command's directory to STAR_RESOURCEPATH environment variable
	ByteString aResPath( getenv( "STAR_RESOURCEPATH" ) );
	if ( aCmdPath.Len() )
	{
		ByteString aTmpPath( "STAR_RESOURCEPATH=" );
		aTmpPath += aCmdPath;
		if ( aResPath.Len() )
		{
			aTmpPath += ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_UTF8 );
			aTmpPath += aResPath;
		}
		putenv( aTmpPath.GetBuffer() );
	}

	// Assign command's directory to DYLD_LIBRARY_PATH environment variable
	ByteString aLibPath( getenv( "DYLD_LIBRARY_PATH" ) );
	if ( aCmdPath.Len() )
	{
		ByteString aTmpPath( "DYLD_LIBRARY_PATH=" );
		aTmpPath += aCmdPath;
		if ( aLibPath.Len() )
		{
			aTmpPath += ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_UTF8 );
			aTmpPath += aLibPath;
		}
		putenv( aTmpPath.GetBuffer() );
		// Restart if necessary since most library path changes don't have
		// any affect after the application has already started on most
		// platforms
		if ( aLibPath.GetToken( 0, ByteString( DirEntry::GetSearchDelimiter(), RTL_TEXTENCODING_UTF8 ).GetChar( 0 ) ).CompareTo( aCmdPath, aCmdPath.Len() ) != COMPARE_EQUAL )
			execv( pCmdPath, argv );
	}

	// Test the JVM version and if it is 1.4 or higher, run SVMain() in a
	// high priority thread
	java_lang_Class* pClass = java_lang_Class::forName( OUString::createFromAscii( "java/lang/CharSequence" ) );
	if ( pClass )
	{
		delete pClass;

		// Load Cocoa
		OModule aModule;
		aModule.load( OUString::createFromAscii( "/System/Library/Frameworks/AppKit.framework/AppKit" ) );

		// Create the SVMain() thread
		SVMainThread aThread;
		aThread.create();

		// Start the Cocoa event loop
		RunCocoaEventLoop();
		aThread.join();
	}
	else
	{
		// Panther expects applications to run their event loop in main thread
		// and Java 1.3.1 runs its event loop in a separate thread. So, we need
		// to disable the lock that Java 1.3.1 uses and simultaneously run an
		// event loop in this thread.
		VCLThreadAttach t;
		if ( t.pEnv )
		{
			jclass carbonLockClass = t.pEnv->FindClass( "com/apple/mrj/macos/carbon/CarbonLock" );
			if ( carbonLockClass )
			{
				// Create the lock object
				pClass = java_lang_Class::forName( OUString::createFromAscii( "java/lang/Object" ) );
				pCarbonLockObject = new java_lang_Object( pClass->newInstanceObject() );
				delete pClass;

				// Reregister the native methods
				JNINativeMethod pMethods[4];
				pMethods[0].name = "acquire0";
				pMethods[0].signature = "()I";
				pMethods[0].fnPtr = Java_com_apple_mrj_macos_carbon_CarbonLock_acquire0;
				pMethods[1].name = "getInstance";
				pMethods[1].signature = "()Ljava/lang/Object;";
				pMethods[1].fnPtr = Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance;
				pMethods[2].name = "init";
				pMethods[2].signature = "()V";
				pMethods[2].fnPtr = Java_com_apple_mrj_macos_carbon_CarbonLock_init;
				pMethods[3].name = "release0";
				pMethods[3].signature = "()I";
				pMethods[3].fnPtr = Java_com_apple_mrj_macos_carbon_CarbonLock_release0;
				t.pEnv->RegisterNatives( carbonLockClass, pMethods, 4 );
			}
		}

		SVMain();
	}
#else	// MACOSX
	SVMain();
#endif	// MACOSX

	exit( 0 );
}

END_C
