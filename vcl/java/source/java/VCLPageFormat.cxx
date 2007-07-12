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
 *  Copyright 2003 Planamesa Inc.
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

#define _SV_COM_SUN_STAR_VCL_VCLPAGEFORMAT_CXX

#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALFRAME_H
#include <salframe.h>
#endif
#ifndef _SV_SVAPP_HXX
#include <svapp.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLPAGEFORMAT_HXX
#include <com/sun/star/vcl/VCLPageFormat.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFRAME_HXX
#include <com/sun/star/vcl/VCLFrame.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_JAVA_LANG_CLASS_HXX
#include <java/lang/Class.hxx>
#endif
#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif

#include "VCLPageFormat_cocoa.h"

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

#define PAGEFORMAT_KEY CFSTR( "PAGEFORMAT" )

typedef void Java_apple_awt_CPrinterJob_getDefaultPage_Type( JNIEnv *, jobject, jobject );
typedef void Java_apple_awt_CPrinterJob_printLoop_Type( JNIEnv *, jobject, jboolean );
typedef jboolean Java_apple_awt_CPrinterJob_printLoopBoolean_Type( JNIEnv *, jobject, jboolean, jint, jint );

static ::vos::OModule aModule;
static Java_apple_awt_CPrinterJob_getDefaultPage_Type *pGetDefaultPage = NULL;
static Java_apple_awt_CPrinterJob_printLoop_Type *pPrintLoop = NULL;
static Java_apple_awt_CPrinterJob_printLoopBoolean_Type *pPrintLoopBoolean = NULL;

using namespace rtl;
using namespace vcl;

// ============================================================================

static void *GetNSPrintInfo( JNIEnv *pEnv, jobject object )
{
	void *out = NULL;

	jclass tempClass = pEnv->FindClass( "apple/awt/CPrinterJob" );
	if ( tempClass && pEnv->IsInstanceOf( object, tempClass ) )
	{
		static jmethodID mIDGetNSPrintInfo = NULL;
		static bool bReturnsInt = false;
		if ( !mIDGetNSPrintInfo )
		{
			char *cSignature = "()J";
			mIDGetNSPrintInfo = pEnv->GetMethodID( tempClass, "getNSPrintInfo", cSignature );
			if ( !mIDGetNSPrintInfo )
			{
				// Java 1.4.1 has a different signature so check for it if
				// we cannot find the first signature
				if ( pEnv->ExceptionCheck() )
					pEnv->ExceptionClear();
				cSignature = "()I";
				mIDGetNSPrintInfo = pEnv->GetMethodID( tempClass, "getNSPrintInfo", cSignature );
				if ( mIDGetNSPrintInfo )
					bReturnsInt = true;
			}
		}
		OSL_ENSURE( mIDGetNSPrintInfo, "Unknown method id!" );
		if ( mIDGetNSPrintInfo )
		{
			if ( bReturnsInt )
				out = (void *)pEnv->CallIntMethod( object, mIDGetNSPrintInfo );
			else
				out = (void *)pEnv->CallLongMethod( object, mIDGetNSPrintInfo );
		}
	}

	return out;
}

// ----------------------------------------------------------------------------

static jlong JNICALL Java_apple_awt_CPrinterJob_createNSPrintInfoLong( JNIEnv *pEnv, jobject object )
{
	return (jlong)NSPrintInfo_create();
}

// ----------------------------------------------------------------------------

static jint JNICALL Java_apple_awt_CPrinterJob_createNSPrintInfoInt( JNIEnv *pEnv, jobject object )
{
	return (jint)NSPrintInfo_create();
}

// ----------------------------------------------------------------------------

static void JNICALL Java_apple_awt_CPrinterJob_getDefaultPage( JNIEnv *pEnv, jobject object, jobject _par0 )
{
	if ( pGetDefaultPage )
	{
		// Make this object's print info pointer the shared print info since
		// the JVM's native methods use the shared print info
		NSPrintInfo_setSharedPrintInfo( GetNSPrintInfo( pEnv, object ) );

		pGetDefaultPage( pEnv, object, _par0 );
	}
}

// ----------------------------------------------------------------------------

static void JNICALL Java_apple_awt_CPrinterJob_printLoop( JNIEnv *pEnv, jobject object, jboolean _par0 )
{
	if ( pPrintLoop )
	{
		// Make this object's print info pointer the shared print info since
		// the JVM's native methods use the shared print info
		NSPrintInfo_setSharedPrintInfo( GetNSPrintInfo( pEnv, object ) );

		pPrintLoop( pEnv, object, _par0 );
	}
}

// ----------------------------------------------------------------------------

static jboolean JNICALL Java_apple_awt_CPrinterJob_printLoopBoolean( JNIEnv *pEnv, jobject object, jboolean _par0, jint _par1, jint _par2 )
{
	jboolean bRet = JNI_FALSE;

	if ( pPrintLoop )
	{
		// Make this object's print info pointer the shared print info since
		// the JVM's native methods use the shared print info
		NSPrintInfo_setSharedPrintInfo( GetNSPrintInfo( pEnv, object ) );

		bRet = pPrintLoopBoolean( pEnv, object, _par0, _par1, _par2 );
	}

	return bRet;
}

// ----------------------------------------------------------------------------

static void JNICALL Java_apple_awt_CPrinterJob_validatePaper( JNIEnv *pEnv, jobject object, jobject _par0, jobject _par1 )
{
	void *pInfo = GetNSPrintInfo( pEnv, object );
	if ( pInfo )
	{
		// Make this object's print info pointer the shared print info since
		// the JVM's native methods use the shared print info
		NSPrintInfo_setSharedPrintInfo( pInfo );

		// Fix bug 2263 by forcing the paper to match the paper dimensions
		// already set in the shared print info
		float fWidth = 0;
		float fHeight = 0;
		float fImageableX = 0;
		float fImageableY = 0;
		float fImageableWidth = 0;
		float fImageableHeight = 0;
		NSPrintInfo_getPrintInfoDimensions( pInfo, &fWidth, &fHeight, &fImageableX, &fImageableY, &fImageableWidth, &fImageableHeight );
		if ( fWidth > 0 && fHeight > 0 && fImageableWidth > 0 && fImageableHeight > 0 )
		{
			static jmethodID mID = NULL;
			if ( !mID )
			{
				char *cSignature = "(Ljava/awt/print/Paper;FFFFFF)V";
				mID = pEnv->GetStaticMethodID( com_sun_star_vcl_VCLPageFormat::getMyClass(), "validatePaper", cSignature );
			}
			OSL_ENSURE( mID, "Unknown method id!" );
			if ( mID )
			{
				jvalue args[7];
				args[0].l = _par1;
				args[1].f = jfloat( fWidth );
				args[2].f = jfloat( fHeight );
				args[3].f = jfloat( fImageableX );
				args[4].f = jfloat( fImageableY );
				args[5].f = jfloat( fImageableWidth );
				args[6].f = jfloat( fImageableHeight );
				pEnv->CallStaticVoidMethodA( com_sun_star_vcl_VCLPageFormat::getMyClass(), mID, args );
			}
		}
	}
}

// ============================================================================

jclass com_sun_star_vcl_VCLPageFormat::theClass = NULL;

// ----------------------------------------------------------------------------

jclass com_sun_star_vcl_VCLPageFormat::getMyClass()
{
	if ( !theClass )
	{
		VCLThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;

		// Fix bug 991 by inserting our own Cocoa class in place of the
		// NSPrintInfo class. We need to do this because the JVM does not use
		// the printer job's cached print info instance. Instead, the JVM
		// always make a copy of the shared print info. So, if the selected
		// printer has changed in a dialog, the JVM will print to the wrong
		// printer if the default printer is set to a specific printer in the
		// printer preferences in the System Preferences application.
		VCLPrintInfo_installVCLPrintClasses();

		// Cache existing functions from libawt.jnilib
		jclass systemClass = t.pEnv->FindClass( "java/lang/System" );
		if ( systemClass )
		{
			// Find libawt.jnilib
			jmethodID mID = NULL;
			OUString aJavaHomePath;
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
				jstring out = (jstring)t.pEnv->CallStaticObjectMethodA( systemClass, mID, args );
				if ( out )
					aJavaHomePath = JavaString2String( t.pEnv, out );
			}

			if ( aJavaHomePath.getLength() )
			{
				OUString aAWTPath( aJavaHomePath );
				aAWTPath += OUString::createFromAscii( "/../Libraries/libawt.jnilib" );
				if ( aModule.load( aAWTPath ) )
				{
					pGetDefaultPage = (Java_apple_awt_CPrinterJob_getDefaultPage_Type *)aModule.getSymbol( OUString::createFromAscii( "Java_apple_awt_CPrinterJob_getDefaultPage" ) );
					pPrintLoop = (Java_apple_awt_CPrinterJob_printLoop_Type *)aModule.getSymbol( OUString::createFromAscii( "Java_apple_awt_CPrinterJob_printLoop" ) );
					pPrintLoopBoolean = (Java_apple_awt_CPrinterJob_printLoopBoolean_Type *)aModule.getSymbol( OUString::createFromAscii( "Java_apple_awt_CPrinterJob_printLoop" ) );
				}
			}
		}

		// Override the CPrinterJob.createNSPrintInfo() method to explicity
		// create a new print info instance instead of copying the shared
		// print info instance
		jclass cPrinterJobClass = t.pEnv->FindClass( "apple/awt/CPrinterJob" );
		if ( cPrinterJobClass )
		{
			JNINativeMethod aMethod;
			aMethod.name = "createNSPrintInfo";
			aMethod.signature = "()J";
			aMethod.fnPtr = (void *)Java_apple_awt_CPrinterJob_createNSPrintInfoLong;
			t.pEnv->RegisterNatives( cPrinterJobClass, &aMethod, 1 );

			// Java 1.4.1 has a different signature for this method so check
			// for it if an exception is thrown
			if ( t.pEnv->ExceptionCheck() )
			{
				t.pEnv->ExceptionClear();
				aMethod.signature = "()I";
				aMethod.fnPtr = (void *)Java_apple_awt_CPrinterJob_createNSPrintInfoInt;
				t.pEnv->RegisterNatives( cPrinterJobClass, &aMethod, 1 );
			}

			JNINativeMethod pMethods[3];
			pMethods[0].name = "getDefaultPage";
			pMethods[0].signature = "(Ljava/awt/print/PageFormat;)V";
			pMethods[0].fnPtr = (void *)Java_apple_awt_CPrinterJob_getDefaultPage;
			pMethods[1].name = "printLoop";
			pMethods[1].signature = "(Z)V";
			pMethods[1].fnPtr = (void *)Java_apple_awt_CPrinterJob_printLoop;
			pMethods[2].name = "validatePaper";
			pMethods[2].signature = "(Ljava/awt/print/Paper;Ljava/awt/print/Paper;)V";
			pMethods[2].fnPtr = (void *)Java_apple_awt_CPrinterJob_validatePaper;
			t.pEnv->RegisterNatives( cPrinterJobClass, pMethods, 3 );

			// Java on Mac OS X 10.4 has a different signature for some methods
			// so check for it if an exception is thrown
			if ( t.pEnv->ExceptionCheck() )
			{
				t.pEnv->ExceptionClear();
				pMethods[1].signature = "(ZII)Z";
				pMethods[1].fnPtr = (void *)Java_apple_awt_CPrinterJob_printLoopBoolean;
				t.pEnv->RegisterNatives( cPrinterJobClass, pMethods, 3 );
			}
		}

		jclass tempClass = t.pEnv->FindClass( "com/sun/star/vcl/VCLPageFormat" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLPageFormat::com_sun_star_vcl_VCLPageFormat() : java_lang_Object( (jobject)NULL )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( !t.pEnv )
		return;
	if ( !mID )
	{
		char *cSignature = "()V";
		mID = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );
	}
	OSL_ENSURE( mID, "Unknown method id!" );
	jobject tempObj;
	tempObj = t.pEnv->NewObject( getMyClass(), mID );
	saveRef( tempObj );
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLPageFormat::dispose()
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()V";
			mID = t.pEnv->GetMethodID( getMyClass(), "dispose", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			t.pEnv->CallNonvirtualVoidMethod( object, getMyClass(), mID );
	}
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLGraphics *com_sun_star_vcl_VCLPageFormat::getGraphics()
{
	static jmethodID mID = NULL;
	com_sun_star_vcl_VCLGraphics *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Lcom/sun/star/vcl/VCLGraphics;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getGraphics", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
				out = new com_sun_star_vcl_VCLGraphics( tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

const Rectangle com_sun_star_vcl_VCLPageFormat::getImageableBounds()
{ 
	static jmethodID mID = NULL;
	static jfieldID fIDX = NULL;
	static jfieldID fIDY = NULL;
	static jfieldID fIDWidth = NULL;
	static jfieldID fIDHeight = NULL;
	Rectangle out( Point( 0, 0 ), Size( 0, 0 ) );
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/awt/Rectangle;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getImageableBounds", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
			{
				jclass tempObjClass = t.pEnv->GetObjectClass( tempObj );
				if ( !fIDX )
				{
					char *cSignature = "I";
					fIDX = t.pEnv->GetFieldID( tempObjClass, "x", cSignature );
				}
				OSL_ENSURE( fIDX, "Unknown field id!" );
				if ( !fIDY )
				{
					char *cSignature = "I";
					fIDY = t.pEnv->GetFieldID( tempObjClass, "y", cSignature );
				}
				OSL_ENSURE( fIDY, "Unknown field id!" );
				if ( !fIDWidth )
				{
					char *cSignature = "I";
					fIDWidth = t.pEnv->GetFieldID( tempObjClass, "width", cSignature );
				}
				OSL_ENSURE( fIDWidth, "Unknown field id!" );
				if ( !fIDHeight )
				{
					char *cSignature = "I";
					fIDHeight = t.pEnv->GetFieldID( tempObjClass, "height", cSignature );
				}
				OSL_ENSURE( fIDHeight, "Unknown field id!" );
				if ( fIDX && fIDY && fIDWidth && fIDHeight )
				{
					Point aPoint( (long)t.pEnv->GetIntField( tempObj, fIDX ), (long)t.pEnv->GetIntField( tempObj, fIDY ) );
					Size aSize( (long)t.pEnv->GetIntField( tempObj, fIDWidth ), (long)t.pEnv->GetIntField( tempObj, fIDHeight ) );
					out = Rectangle( aPoint, aSize );
				}
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

void *com_sun_star_vcl_VCLPageFormat::getNativePrinterJob()
{
	void *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		java_lang_Object *printerJob = getPrinterJob();
		if ( printerJob )
		{
			jobject tempObj = printerJob->getJavaObject();
			if ( tempObj )
				out = GetNSPrintInfo( t.pEnv, tempObj );
			delete printerJob;
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

Orientation com_sun_star_vcl_VCLPageFormat::getOrientation()
{
	static jmethodID mID = NULL;
	Orientation out = ORIENTATION_PORTRAIT;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getOrientation", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (Orientation)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

const Size com_sun_star_vcl_VCLPageFormat::getPageSize()
{ 
	static jmethodID mID = NULL;
	static jfieldID fIDWidth = NULL;
	static jfieldID fIDHeight = NULL;
	Size out( 0, 0 );
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/awt/Dimension;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getPageSize", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
			{
				jclass tempObjClass = t.pEnv->GetObjectClass( tempObj );
				if ( !fIDWidth )
				{
					char *cSignature = "I";
					fIDWidth = t.pEnv->GetFieldID( tempObjClass, "width", cSignature );
				}
				OSL_ENSURE( fIDWidth, "Unknown field id!" );
				if ( !fIDHeight )
				{
					char *cSignature = "I";
					fIDHeight = t.pEnv->GetFieldID( tempObjClass, "height", cSignature );
				}
				OSL_ENSURE( fIDHeight, "Unknown field id!" );
				if ( fIDWidth && fIDHeight )
					out = Size( (long)t.pEnv->GetIntField( tempObj, fIDWidth ), (long)t.pEnv->GetIntField( tempObj, fIDHeight ) );
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

java_lang_Object *com_sun_star_vcl_VCLPageFormat::getPrinterJob()
{
	static jmethodID mID = NULL;
	java_lang_Object *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/awt/print/PrinterJob;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getPrinterJob", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
				out = new java_lang_Object( tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

Paper com_sun_star_vcl_VCLPageFormat::getPaperType()
{
	static jmethodID mID = NULL;
	Paper out = PAPER_USER;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getPaperType", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (Paper)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

const Size com_sun_star_vcl_VCLPageFormat::getTextResolution()
{ 
	static jmethodID mID = NULL;
	static jfieldID fIDWidth = NULL;
	static jfieldID fIDHeight = NULL;
	Size out( 0, 0 );
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/awt/Dimension;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getTextResolution", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
			{
				jclass tempObjClass = t.pEnv->GetObjectClass( tempObj );
				if ( !fIDWidth )
				{
					char *cSignature = "I";
					fIDWidth = t.pEnv->GetFieldID( tempObjClass, "width", cSignature );
				}
				OSL_ENSURE( fIDWidth, "Unknown field id!" );
				if ( !fIDHeight )
				{
					char *cSignature = "I";
					fIDHeight = t.pEnv->GetFieldID( tempObjClass, "height", cSignature );
				}
				OSL_ENSURE( fIDHeight, "Unknown field id!" );
				if ( fIDWidth && fIDHeight )
					out = Size( (long)t.pEnv->GetIntField( tempObj, fIDWidth ), (long)t.pEnv->GetIntField( tempObj, fIDHeight ) );
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLPageFormat::isEditable()
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "isEditable", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			out = (sal_Bool)t.pEnv->CallNonvirtualBooleanMethod( object, getMyClass(), mID );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLPageFormat::setOrientation( Orientation _par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(I)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setOrientation", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].i = jint( _par0 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLPageFormat::setup()
{
	sal_Bool out = sal_False;

	if (!isEditable())
		return out;

	SalData *pSalData = GetSalData();

	JavaSalFrame *pFocusFrame = pSalData->mpFocusFrame;
	if ( pFocusFrame )
	{
		updatePageFormat( ORIENTATION_PORTRAIT );

		// Ignore any AWT events while the page layout dialog is showing to
		// emulate a modal dialog
		void *pNSPrintInfo = getNativePrinterJob();
		void *pDialog = NSPrintInfo_showPageLayoutDialog( pNSPrintInfo, pFocusFrame->mpVCLFrame->getNativeWindow(), ( getOrientation() == ORIENTATION_LANDSCAPE ) ? TRUE : FALSE );

		pSalData->mpNativeModalSheetFrame = pFocusFrame;
		pSalData->mbInNativeModalSheet = true;
		while ( !NSPageLayout_finished( pDialog ) )
			Application::Yield();
		pSalData->mbInNativeModalSheet = false;
		pSalData->mpNativeModalSheetFrame = NULL;

		out = (sal_Bool)NSPageLayout_result( pDialog );

		if ( out )
			updatePageFormat( ORIENTATION_PORTRAIT );
	}

	return out;
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLPageFormat::setPaperType( Paper _par0, long _par1, long _par2 )
{
	if (!isEditable())
		return;

	void *pNSPrintInfo = getNativePrinterJob();

	if ( _par0 == PAPER_A3 )
	{
		_par1 = 842;
		_par2 = 1191;
	}
	else if ( _par0 == PAPER_A4 )
	{
		_par1 = 595;
		_par2 = 842;
	}
	else if ( _par0 == PAPER_A5 )
	{
		_par1 = 420;
		_par2 = 595;
	}
	else if ( _par0 == PAPER_B4 )
	{
		_par1 = 709;
		_par2 = 1001;
	}
	else if ( _par0 == PAPER_B5 )
	{
		_par1 = 499;
		_par2 = 709;
	}
	else if ( _par0 == PAPER_LETTER )
	{
		_par1 = 612;
		_par2 = 792;
	}
	else if ( _par0 == PAPER_LEGAL )
	{
		_par1 = 612;
		_par2 = 1008;
	}
	else if ( _par0 == PAPER_TABLOID )
	{
		_par1 = 792;
		_par2 = 1224;
	}

	BOOL bLandscape = NSPrintInfo_setPaperSize( pNSPrintInfo, _par1, _par2 );

	updatePageFormat( bLandscape ? ORIENTATION_LANDSCAPE : ORIENTATION_PORTRAIT );
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLPageFormat::updatePageFormat( Orientation _par0 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(I)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "updatePageFormat", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].i = jint( _par0 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}
