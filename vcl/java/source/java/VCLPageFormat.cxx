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

#define _SV_COM_SUN_STAR_VCL_VCLPAGEFORMAT_CXX

#ifndef _SV_COM_SUN_STAR_VCL_VCLPAGEFORMAT_HXX
#include <com/sun/star/vcl/VCLPageFormat.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_JAVA_LANG_CLASS_HXX
#include <java/lang/Class.hxx>
#endif

#ifdef MACOSX

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

#define PAGEFORMAT_KEY CFSTR( "PAGEFORMAT" )

using namespace rtl;

#endif	// MACOSX

using namespace vcl;

// ============================================================================

#ifdef MACOSX
static jint JNICALL Java_com_apple_mrj_internal_awt_printing_MacPageFormat_createBestFormat( JNIEnv *pEnv, jobject object, jint pSessionPtr )
{
	jint nRet = 0;
	PMPrintSession pSession = (PMPrintSession)pSessionPtr;
	if ( pSession )
	{
		CFNumberRef aData = NULL;
		if ( PMSessionGetDataFromSession( pSession, PAGEFORMAT_KEY, (CFTypeRef *)&aData ) == kPMNoError && aData )
		{
			CFIndex aValue;
			if ( CFNumberGetValue( aData, kCFNumberCFIndexType, &aValue ) )
			{
				PMPageFormat aPageFormat = (PMPageFormat)aValue;
				if ( aPageFormat && PMRetain( aPageFormat ) == kPMNoError )
					nRet = (jint)aPageFormat;
			}
		}
	}
	return nRet;
}
#endif	// MACOSX

// ============================================================================

jclass com_sun_star_vcl_VCLPageFormat::theClass = NULL;

// ----------------------------------------------------------------------------

jclass com_sun_star_vcl_VCLPageFormat::getMyClass()
{
	if ( !theClass )
	{
		VCLThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;

#ifdef MACOSX
		// Test the JVM version and if it is below 1.4, use Carbon printing APIs
		if ( t.pEnv->GetVersion() < JNI_VERSION_1_4 )
		{
			// We need to replace the native MacPageFormat.createBestFormat()
			// method as it will throw exceptions whenever a user selects a
			// custom page format
			jclass pageFormatClass = t.pEnv->FindClass( "com/apple/mrj/internal/awt/printing/MacPageFormat" );
			if ( pageFormatClass )
			{
				JNINativeMethod aMethod;
				aMethod.name = "createBestFormat";
				aMethod.signature = "(I)I";
				aMethod.fnPtr = (void *)Java_com_apple_mrj_internal_awt_printing_MacPageFormat_createBestFormat;
				t.pEnv->RegisterNatives( pageFormatClass, &aMethod, 1 );
			}
		}
#endif	// MACOSX

		jclass tempClass = t.pEnv->FindClass( "com/sun/star/vcl/VCLPageFormat" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLPageFormat::com_sun_star_vcl_VCLPageFormat() : java_lang_Object( (jobject)NULL ), mbInitialized( FALSE )
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
	initializeNativePrintJob();
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLPageFormat::destroyNativePrintJob()
{
	if ( !mbInitialized )
		return;

#ifdef MACOSX
	PMPrintSession pSession = (PMPrintSession)getNativePrintJob();
	if ( pSession )
	{
		CFNumberRef aData = NULL;
		if ( PMSessionGetDataFromSession( pSession, PAGEFORMAT_KEY, (CFTypeRef *)&aData ) == kPMNoError && aData )
		{
			CFIndex aValue;
			if ( CFNumberGetValue( aData, kCFNumberCFIndexType, &aValue ) )
			{
				PMPageFormat aPageFormat = (PMPageFormat)aValue;
				if ( aPageFormat )
					PMRelease( aPageFormat );
			}
		}
		mbInitialized = FALSE;
	}
#endif	// MACOSX
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

void *com_sun_star_vcl_VCLPageFormat::getNativePrintJob()
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
			{
#ifdef MACOSX
				// Test the JVM version and if it is below 1.4, use Carbon
				// printing APIs
				if ( t.pEnv->GetVersion() < JNI_VERSION_1_4 )
				{
					jclass tempClass = t.pEnv->FindClass( "com/apple/mrj/internal/awt/printing/MacPrinterJob" );
					if ( tempClass && t.pEnv->IsInstanceOf( tempObj, tempClass ) )
					{
						static jfieldID fIDSession = NULL;
						if ( !fIDSession )
						{
							char *cSignature = "Lcom/apple/mrj/macos/generated/PMPrintSessionOpaque;";
							fIDSession = t.pEnv->GetFieldID( tempClass, "fPrintSession", cSignature );
						}
						OSL_ENSURE( fIDSession, "Unknown field id!" );
						if ( fIDSession )
						{
							jobject session = t.pEnv->GetObjectField( tempObj, fIDSession );
							if ( session )
							{
								static jmethodID mIDGetPointer = NULL;
								jclass sessionClass = t.pEnv->GetObjectClass( session );
								if ( !mIDGetPointer )
								{
									char *cSignature = "()I";
									mIDGetPointer = t.pEnv->GetMethodID( sessionClass, "getPointer", cSignature );
								}
								OSL_ENSURE( mIDGetPointer, "Unknown method id!" );
								if ( mIDGetPointer )
									out = (void *)t.pEnv->CallIntMethod( session, mIDGetPointer );
							}
						}
					}
				}
#endif	// MACOSX
			}
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

void com_sun_star_vcl_VCLPageFormat::initializeNativePrintJob()
{
	if ( mbInitialized )
		return;

#ifdef MACOSX
	PMPrintSession pSession = (PMPrintSession)getNativePrintJob();
	if ( pSession )
	{
		PMPageFormat aPageFormat = NULL;
		CFNumberRef aData = NULL;
		if ( PMSessionGetDataFromSession( pSession, PAGEFORMAT_KEY, (CFTypeRef *)&aData ) == kPMNoError && aData )
		{
			CFIndex aValue;
			if ( CFNumberGetValue( aData, kCFNumberCFIndexType, &aValue ) )
			{
				aPageFormat = (PMPageFormat)aValue;
				if ( PMRetain( aPageFormat ) != kPMNoError )
				{
					PMRelease( aPageFormat );
					aPageFormat = NULL;
				}
			}
		}
		if ( !aPageFormat )
		{
			if ( PMCreatePageFormat( &aPageFormat ) == kPMNoError )
			{
				if ( PMSessionDefaultPageFormat( pSession, aPageFormat ) == kPMNoError && ( aData = CFNumberCreate( kCFAllocatorDefault,  kCFNumberCFIndexType, &aPageFormat ) ) != NULL )
				{
					PMSessionSetDataInSession( pSession, PAGEFORMAT_KEY, (CFTypeRef)aData );
				}
				else
				{
					PMRelease( aPageFormat );
					aPageFormat = NULL;
				}
			}
		}
		if ( aPageFormat )
			mbInitialized = TRUE;
	}
#endif	// MACOSX
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLPageFormat::resetPageResolution()
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()V";
			mID = t.pEnv->GetMethodID( getMyClass(), "resetPageResolution", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			t.pEnv->CallNonvirtualVoidMethod( object, getMyClass(), mID );
	}
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

void com_sun_star_vcl_VCLPageFormat::setPageResolution( long _par0, long _par1 )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(II)V";
			mID = t.pEnv->GetMethodID( getMyClass(), "setPageResolution", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[2];
			args[0].i = jint( _par0 );
			args[1].i = jint( _par1 );
			t.pEnv->CallNonvirtualVoidMethodA( object, getMyClass(), mID, args );
		}
	}
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLPageFormat::setup()
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "setup", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (sal_Bool)t.pEnv->CallNonvirtualBooleanMethod( object, getMyClass(), mID );
	}
	return out;
}
