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
#ifndef _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
#include <com/sun/star/vcl/VCLFont.hxx>
#endif
#ifndef _SV_JAVA_LANG_CLASS_HXX 
#include <java/lang/Class.hxx>
#endif
#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif
#include <crt_externs.h>
#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

typedef jobject Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance_Type( JNIEnv *, jobject );
typedef void Java_com_apple_mrj_macos_carbon_CarbonLock_init_Type( JNIEnv *, jobject );

struct SVNativeFontList
{
	ATSFontRef				maFont;
	SVNativeFontList*		mpNext;
};

class SVMainThread : public ::vos::OThread
{
protected:
	virtual void			run() { SVMain(); _exit( 0 ); }
};

using namespace osl;
using namespace rtl;
using namespace vcl;
using namespace vos;

static Mutex aMutex;
static OModule aJDirectModule;
static Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance_Type *pCarbonLockGetInstance = NULL;
static Java_com_apple_mrj_macos_carbon_CarbonLock_init_Type *pCarbonLockInit = NULL;
static SVNativeFontList *pNativeFontList = NULL;

static jobject JNICALL Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance( JNIEnv *pEnv, jobject object );

#endif

// ============================================================================

#ifdef MACOSX
static void ImplFontListChangedCallback( ATSFontNotificationInfoRef, void* )
{
	MutexGuard aGuard( aMutex );

	// Get the array of fonts
	SVNativeFontList *pNewFontList = NULL;
	BOOL bContinue = TRUE;
	while ( bContinue )
	{
		ATSFontIterator aIterator;
		ATSFontRef aFont;
		ATSFontIteratorCreate( kATSFontContextLocal, NULL, NULL, kATSOptionFlagsUnRestrictedScope, &aIterator );
		for ( ; ; )
		{
			OSStatus nErr = ATSFontIteratorNext( aIterator, &aFont );
			if ( nErr == kATSIterationCompleted )
			{
				bContinue = FALSE;
				break;
			}
			else if ( nErr == kATSIterationScopeModified )
			{
				while ( pNewFontList )
				{
					SVNativeFontList *pFont = pNewFontList;
					pNewFontList = pFont->mpNext;
					delete pFont;
				}
				break;
			}
			else
			{
				SVNativeFontList *pFont = new SVNativeFontList();
				pFont->maFont = aFont;
				pFont->mpNext = pNewFontList;
				pNewFontList = pFont;
			}
		}
		ATSFontIteratorRelease( &aIterator );
	}

	// If any of the Java fonts have been disabled, use the default font
	BOOL bUseDefaultFont = FALSE;
	SVNativeFontList *pCurrentFont = pNativeFontList;
	while ( pCurrentFont )
	{
		SVNativeFontList *pNewFont = pNewFontList;
		BOOL bFound = FALSE;
		while ( pNewFont )
		{
			if ( pCurrentFont->maFont == pNewFont->maFont )
			{
				bFound = TRUE;
				break;
			}
			else
			{
				pNewFont = pNewFont->mpNext;
			}
		}

		if ( !bFound )
		{
			bUseDefaultFont = TRUE;
			break;
		}

		pCurrentFont = pCurrentFont->mpNext;
	}

	com_sun_star_vcl_VCLFont::useDefaultFont = bUseDefaultFont;

	// Release the temporary font list
	while ( pNewFontList )
	{
		SVNativeFontList *pFont = pNewFontList;
		pNewFontList = pFont->mpNext;
		delete pFont;
	}
}
#endif	// MACOSX

// ----------------------------------------------------------------------------

#ifdef MACOSX
static jint JNICALL Java_com_apple_mrj_macos_carbon_CarbonLock_acquire0( JNIEnv *pEnv, jobject object )
{
	jobject lockObject = Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance( pEnv, object );
	if ( lockObject )
	{
		jint nRet = pEnv->MonitorEnter( lockObject );
		if ( pEnv->ExceptionOccurred() )
		{
			pEnv->ExceptionDescribe();
			pEnv->ExceptionClear();
		}
		return nRet == JNI_OK ? 0 : 1;
	}
	else
	{
		return 1;
	}
}
#endif

// ----------------------------------------------------------------------------

#ifdef MACOSX
static jobject JNICALL Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance( JNIEnv *pEnv, jobject object )
{
	if ( pCarbonLockGetInstance )
		return pCarbonLockGetInstance( pEnv, object );
	else
		return NULL;
}
#endif

// ----------------------------------------------------------------------------

#ifdef MACOSX
static void JNICALL Java_com_apple_mrj_macos_carbon_CarbonLock_init( JNIEnv *pEnv, jobject object )
{
	if ( pCarbonLockInit )
		pCarbonLockInit( pEnv, object );
}
#endif

// ----------------------------------------------------------------------------

#ifdef MACOSX
static jint JNICALL Java_com_apple_mrj_macos_carbon_CarbonLock_release0( JNIEnv *pEnv, jobject object )
{
	jobject lockObject = Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance( pEnv, object );
	if ( lockObject )
	{
		jint nRet = pEnv->MonitorExit( lockObject );
		if ( pEnv->ExceptionOccurred() )
		{
			pEnv->ExceptionDescribe();
			pEnv->ExceptionClear();
		}
		return nRet == JNI_OK ? 0 : 1;
	}
	else
	{
		return 1;
	}
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

	// If there is a "share/fonts/truetype" directory, explicitly activate the
	// fonts since Panther does not automatically add fonts in the user's
	// Library/Fonts directory until they reboot or relogin
	if ( aCmdPath.Len() )
	{
		ByteString aFontDir( aCmdPath );
		if ( aFontDir.Len() )
		{
			aFontDir += ByteString( "/../share/fonts/truetype", RTL_TEXTENCODING_UTF8 );
			FSRef aFontPath;
			FSSpec aFontSpec;
			if ( FSPathMakeRef( (const UInt8 *)aFontDir.GetBuffer(), &aFontPath, 0 ) == noErr && FSGetCatalogInfo( &aFontPath, kFSCatInfoNone, NULL, NULL, &aFontSpec, NULL) == noErr )
				ATSFontActivateFromFileSpecification( &aFontSpec, kATSFontContextLocal, kATSFontFormatUnspecified, NULL, kATSOptionFlagsDefault, NULL );
		}
	}

	jint nFonts = 0;
	ATSFontIterator aIterator;
	ATSFontRef aFont;

	// Get the array of fonts
	BOOL bContinue = TRUE;
	while ( bContinue )
	{
		MutexGuard aGuard( aMutex );

		ATSFontIteratorCreate( kATSFontContextLocal, NULL, NULL, kATSOptionFlagsUnRestrictedScope, &aIterator );
		for ( ; ; )
		{
			OSStatus nErr = ATSFontIteratorNext( aIterator, &aFont );
			if ( nErr == kATSIterationCompleted )
			{
				// Register notification callback
				ATSFontNotificationSubscribe( (ATSNotificationCallback)ImplFontListChangedCallback, kATSFontNotifyOptionReceiveWhileSuspended, NULL, NULL );
				bContinue = FALSE;
				break;
			}
			else if ( nErr == kATSIterationScopeModified )
			{
				nFonts = 0;
				while ( pNativeFontList )
				{
					SVNativeFontList *pFont = pNativeFontList;
					pNativeFontList = pFont->mpNext;
					delete pFont;
				}
				break;
			}
			else
			{
				SVNativeFontList *pFont = new SVNativeFontList();
				pFont->maFont = aFont;
				pFont->mpNext = pNativeFontList;
				pNativeFontList = pFont;
				nFonts++;
			}
		}
		ATSFontIteratorRelease( &aIterator );
	}

	// Determine if we were launched from the Finder. If not, force the
	// application to run as a background application.
	if ( argc < 2 || strncmp( argv[1], "-psn_", 5 ) )
	{
		VCLThreadAttach t;
		if ( t.pEnv )
		{
			jclass systemClass = t.pEnv->FindClass( "java/lang/System" );
			if ( systemClass )
			{
				jmethodID mID = NULL;
				OUString aJDirectPath;
				if ( !mID )
				{
					char *cSignature = "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;";
					mID = t.pEnv->GetStaticMethodID( systemClass, "setProperty", cSignature );
				}
				OSL_ENSURE( mID, "Unknown method id!" );
				if ( mID )
				{
					jvalue args[2];
					args[0].l = StringToJavaString( t.pEnv, OUString::createFromAscii( "com.apple.backgroundOnly" ) );
						args[1].l = StringToJavaString( t.pEnv, OUString::createFromAscii( "true" ) );
					(jstring)t.pEnv->CallStaticObjectMethodA( systemClass, mID, args );
				}
			}
		}
	}

	// Test the JVM version and if it is 1.4 or higher, run SVMain() in a
	// high priority thread
	java_lang_Class* pClass = java_lang_Class::forName( OUString::createFromAscii( "java/lang/CharSequence" ) );
	if ( pClass )
	{
		delete pClass;

		// Load Cocoa
		OModule aModule;
		if ( aModule.load( OUString::createFromAscii( "/System/Library/Frameworks/AppKit.framework/AppKit" ) ) )
		{
			// Create the SVMain() thread
			SVMainThread aThread;
			aThread.create();

			// Start the Cocoa event loop
			RunCocoaEventLoop();
			aThread.join();
		}

		return( 0 );
	}
	else
	{
		// Panther expects applications to run their event loop in main thread
		// and Java 1.3.1 runs its event loop in a separate thread. So, we need
		// to disable the lock that Java 1.3.1 uses.
		VCLThreadAttach t;
		if ( t.pEnv )
		{
			jclass systemClass = t.pEnv->FindClass( "java/lang/System" );
			if ( systemClass )
			{
				// Find libJDirect.jnilib
				jmethodID mID = NULL;
				OUString aJDirectPath;
				if ( !mID )
				{
					char *cSignature = "(Ljava/lang/String;)Ljava/lang/String;";
					mID = t.pEnv->GetStaticMethodID( systemClass, "getProperty", cSignature );
				}
				OSL_ENSURE( mID, "Unknown method id!" );
				if ( mID )
				{
					jvalue args[1];
					args[0].l = StringToJavaString( t.pEnv, OUString::createFromAscii( "java.home" ) );
					jstring out;
					out = (jstring)t.pEnv->CallStaticObjectMethodA( systemClass, mID, args );
					if ( out )
						aJDirectPath = JavaString2String( t.pEnv, out );
				}

				// Load libJDirect.jnilib and cache symbols
				if ( aJDirectPath.getLength() )
				{
					aJDirectPath += OUString::createFromAscii( "/../Libraries/libJDirect.jnilib" );
					if ( aJDirectModule.load( aJDirectPath ) )
					{
						pCarbonLockGetInstance = (Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance_Type *)aJDirectModule.getSymbol( OUString::createFromAscii( "Java_com_apple_mrj_macos_carbon_CarbonLock_getInstance" ) );
						pCarbonLockInit = (Java_com_apple_mrj_macos_carbon_CarbonLock_init_Type *)aJDirectModule.getSymbol( OUString::createFromAscii( "Java_com_apple_mrj_macos_carbon_CarbonLock_init" ) );
					}
				}

			}

			jclass carbonLockClass = t.pEnv->FindClass( "com/apple/mrj/macos/carbon/CarbonLock" );
			if ( carbonLockClass && pCarbonLockGetInstance && pCarbonLockInit )
			{
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

				// Peek for a Carbon event. This is enough to solve the
				// keyboard layout switching problem on Panther.
				ReceiveNextEvent( 0, NULL, 0, false, NULL );

				// We need to be fill in the static sFonts and sNumFonts fields 
				// in the NativeFontWrapper class as the JVM's implementation
				// will include disabled fonts will can crash the application
				jclass nativeFontWrapperClass = t.pEnv->FindClass( "sun/awt/font/NativeFontWrapper" );
				if ( nativeFontWrapperClass )
				{
					static jfieldID fIDFonts = NULL;
					static jfieldID fIDNumFonts = NULL;
					if ( !fIDFonts )
					{
						char *cSignature = "[I";
						fIDFonts = t.pEnv->GetStaticFieldID( nativeFontWrapperClass, "sFonts", cSignature );
					}
					OSL_ENSURE( fIDFonts, "Unknown field id!" );
					if ( !fIDNumFonts )
					{
						char *cSignature = "I";
						fIDNumFonts = t.pEnv->GetStaticFieldID( nativeFontWrapperClass, "sNumFonts", cSignature );
					}
					OSL_ENSURE( fIDNumFonts, "Unknown field id!" );
					if ( fIDFonts && fIDNumFonts )
					{
						// Create the font array
						jintArray pFonts = t.pEnv->NewIntArray( nFonts );
						jsize i = 0;
						jboolean bCopy( sal_False );
						jint *pData = t.pEnv->GetIntArrayElements( pFonts, &bCopy );
						ClearableMutexGuard aGuard( aMutex );

						SVNativeFontList *pFont = pNativeFontList;
						while ( pFont )
						{
							pData[i++] = (jint)CGFontCreateWithPlatformFont( (void *)&pFont->maFont );
							pFont = pFont->mpNext;
						}
						t.pEnv->ReleaseIntArrayElements( pFonts, pData, 0 );

						aGuard.clear();

						// Save the font data
						t.pEnv->SetStaticObjectField( nativeFontWrapperClass, fIDFonts, pFonts );
						t.pEnv->SetStaticIntField( nativeFontWrapperClass, fIDNumFonts, nFonts );
					}
				}
			}
		}

		SVMain();

		exit( 0 );
	}
#endif	// MACOSX
	SVMain();

	exit( 0 );
}

END_C
