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

#define _SV_COM_SUN_STAR_VCL_VCLPRINTJOB_CXX

#ifndef _SV_COM_SUN_STAR_VCL_VCLPRINTJOB_HXX
#include <com/sun/star/vcl/VCLPrintJob.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLFRAME_HXX
#include <com/sun/star/vcl/VCLFrame.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLPAGEFORMAT_HXX
#include <com/sun/star/vcl/VCLPageFormat.hxx>
#endif
#ifndef _SV_JAVA_LANG_CLASS_HXX
#include <java/lang/Class.hxx>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifdef MACOSX

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

using namespace rtl;

#endif	// MACOSX

using namespace vcl;

// ============================================================================

jclass com_sun_star_vcl_VCLPrintJob::theClass = NULL;

// ----------------------------------------------------------------------------

jclass com_sun_star_vcl_VCLPrintJob::getMyClass()
{
	if ( !theClass )
	{
		VCLThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;
		jclass tempClass = t.pEnv->FindClass( "com/sun/star/vcl/VCLPrintJob" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLPrintJob::com_sun_star_vcl_VCLPrintJob() : java_lang_Object( (jobject)NULL )
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

void com_sun_star_vcl_VCLPrintJob::abortJob()
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()V";
			mID = t.pEnv->GetMethodID( getMyClass(), "abortJob", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			t.pEnv->CallNonvirtualVoidMethod( object, getMyClass(), mID );
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLPrintJob::dispose()
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

void com_sun_star_vcl_VCLPrintJob::endJob()
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()V";
			mID = t.pEnv->GetMethodID( getMyClass(), "endJob", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			t.pEnv->CallNonvirtualVoidMethod( object, getMyClass(), mID );
	}
}

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLPrintJob::endPage()
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()V";
			mID = t.pEnv->GetMethodID( getMyClass(), "endPage", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			t.pEnv->CallNonvirtualVoidMethod( object, getMyClass(), mID );
	}
}

// ----------------------------------------------------------------------------

void *com_sun_star_vcl_VCLPrintJob::getNativePrintJob()
{
	static jmethodID mID = NULL;
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

XubString com_sun_star_vcl_VCLPrintJob::getPageRange()
{
	static jmethodID mID = NULL;
	XubString out;
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
						jint firstPage = 0;
						jint lastPage = 0;
						static jfieldID fIDFirstPage = NULL;
						static jfieldID fIDLastPage = NULL;
						if ( !fIDFirstPage )
						{
							char *cSignature = "I";
							fIDFirstPage = t.pEnv->GetFieldID( tempClass, "mFirstPage", cSignature );
						}
						OSL_ENSURE( fIDFirstPage, "Unknown field id!" );
						if ( !fIDLastPage )
						{
							char *cSignature = "I";
							fIDLastPage = t.pEnv->GetFieldID( tempClass, "mLastPage", cSignature );
						}
						OSL_ENSURE( fIDLastPage, "Unknown field id!" );
						if ( fIDFirstPage && fIDLastPage )
						{
							firstPage = t.pEnv->GetIntField( tempObj, fIDFirstPage ) + 1;
							lastPage = t.pEnv->GetIntField( tempObj, fIDLastPage ) + 1;
						}
						if ( firstPage > 0 && lastPage > 0 && firstPage <= lastPage && lastPage < 0x7fffffff )
						{
							out = XubString::CreateFromInt32( firstPage );
							out += '-';
							out += XubString::CreateFromInt32( lastPage );
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

java_lang_Object *com_sun_star_vcl_VCLPrintJob::getPrinterJob()
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

sal_Bool com_sun_star_vcl_VCLPrintJob::isFinished()
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "isFinished", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (sal_Bool)t.pEnv->CallNonvirtualBooleanMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLPrintJob::startJob( com_sun_star_vcl_VCLPageFormat *_par0, sal_Bool _par1 ) 
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
#ifdef MACOSX
		// Test the JVM version and if it is below 1.4, use Carbon
		// printing APIs
		if ( t.pEnv->GetVersion() < JNI_VERSION_1_4 )
		{
			// Reset the print dialog to print all pages
			java_lang_Object *printerJob = _par0->getPrinterJob();
			if ( printerJob )
			{
				jobject tempObj = printerJob->getJavaObject();
				if ( tempObj )
				{
					jclass tempClass = t.pEnv->FindClass( "com/apple/mrj/internal/awt/printing/MacPrinterJob" );
					if ( tempClass && t.pEnv->IsInstanceOf( tempObj, tempClass ) )
					{
						static jfieldID fIDPrintSettings = NULL;
						if ( !fIDPrintSettings )
						{
							char *cSignature = "Lcom/apple/mrj/macos/generated/PMPrintSettingsOpaque;";
							fIDPrintSettings = t.pEnv->GetFieldID( tempClass, "fPrintSettings", cSignature );
						}
						OSL_ENSURE( fIDPrintSettings, "Unknown field id!" );
						if ( fIDPrintSettings )
						{
							jobject settings = t.pEnv->GetObjectField( tempObj, fIDPrintSettings );
							if ( settings )
							{
								static jmethodID mIDGetPointer = NULL;
								jclass settingsClass = t.pEnv->GetObjectClass( settings );
								if ( !mIDGetPointer )
								{
									char *cSignature = "()I";
									mIDGetPointer = t.pEnv->GetMethodID( settingsClass, "getPointer", cSignature );
								}
								OSL_ENSURE( mIDGetPointer, "Unknown method id!" );
								if ( mIDGetPointer )
								{
									PMPrintSettings pSettings = (PMPrintSettings)t.pEnv->CallIntMethod( settings, mIDGetPointer );
									if ( pSettings )
										PMSetPageRange( pSettings, 1, kPMPrintAllPages );
								}
							}
						}
					}
				}
			}
			delete printerJob;
		}
#endif	// MACOSX

		// Force the focus frame to the front as Java usually uses the name of
		// the print job from the front-most window
		SalData *pSalData = GetSalData();
		if ( pSalData->mpFocusFrame && pSalData->mpFocusFrame->maFrameData.mbVisible )
			pSalData->mpFocusFrame->maFrameData.mpVCLFrame->toFront();

		if ( !mID )
		{
			char *cSignature = "(Lcom/sun/star/vcl/VCLPageFormat;Z)Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "startJob", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[2];
			args[0].l = _par0->getJavaObject();
			args[1].z = jboolean( _par1 );
			out = (sal_Bool)t.pEnv->CallNonvirtualBooleanMethodA( object, getMyClass(), mID, args );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLGraphics *com_sun_star_vcl_VCLPrintJob::startPage( Orientation _par0 )
{
	static jmethodID mID = NULL;
	com_sun_star_vcl_VCLGraphics *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(I)Lcom/sun/star/vcl/VCLGraphics;";
			mID = t.pEnv->GetMethodID( getMyClass(), "startPage", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[1];
			args[0].i = jint( _par0 );
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethodA( object, getMyClass(), mID, args );
			if ( tempObj )
				out = new com_sun_star_vcl_VCLGraphics( tempObj );
		}
	}
	return out;
}
