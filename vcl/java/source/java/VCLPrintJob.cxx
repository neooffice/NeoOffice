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
#ifndef _SV_COM_SUN_STAR_VCL_VCLGRAPHICS_HXX
#include <com/sun/star/vcl/VCLGraphics.hxx>
#endif
#ifndef _SV_COM_SUN_STAR_VCL_VCLPAGEFORMAT_HXX
#include <com/sun/star/vcl/VCLPageFormat.hxx>
#endif
#ifndef _SV_JAVA_LANG_CLASS_HXX
#include <java/lang/Class.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#include "VCLPrintJob_cocoa.h"

using namespace vcl;

// ============================================================================

JNIEXPORT void JNICALL Java_com_sun_star_vcl_VCLPrintJob_runNativeTimers( JNIEnv *pEnv, jobject object )
{
	NSPrintOperation_runNativeTimers();
}

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

		if ( tempClass )
		{
			// Register the native methods for our class
			JNINativeMethod aMethod; 
			aMethod.name = "runNativeTimers";
			aMethod.signature = "()V";
			aMethod.fnPtr = (void *)Java_com_sun_star_vcl_VCLPrintJob_runNativeTimers;
			t.pEnv->RegisterNatives( tempClass, &aMethod, 1 );
		}

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

void *com_sun_star_vcl_VCLPrintJob::getNativePrinterJob()
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
				jclass tempClass = t.pEnv->FindClass( "apple/awt/CPrinterJob" );
				if ( tempClass && t.pEnv->IsInstanceOf( tempObj, tempClass ) )
				{
					static jmethodID mIDGetNSPrintInfo = NULL;
					static bool bReturnsInt = false;
					if ( !mIDGetNSPrintInfo )
					{
						char *cSignature = "()J";
						mIDGetNSPrintInfo = t.pEnv->GetMethodID( tempClass, "getNSPrintInfo", cSignature );
						if ( !mIDGetNSPrintInfo )
						{
							// Java 1.4.1 has a different signature so check
							// for it if we cannot find the first signature
							if ( t.pEnv->ExceptionCheck() )
								t.pEnv->ExceptionClear();
							cSignature = "()I";
							mIDGetNSPrintInfo = t.pEnv->GetMethodID( tempClass, "getNSPrintInfo", cSignature );
							if ( mIDGetNSPrintInfo )
								bReturnsInt = true;
						}
					}
					OSL_ENSURE( mIDGetNSPrintInfo, "Unknown method id!" );
					if ( mIDGetNSPrintInfo )
					{
						if ( bReturnsInt )
							out = (void *)t.pEnv->CallIntMethod( tempObj, mIDGetNSPrintInfo );
						else
							out = (void *)t.pEnv->CallLongMethod( tempObj, mIDGetNSPrintInfo );
					}
				}
			}
			delete printerJob;
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

XubString com_sun_star_vcl_VCLPrintJob::getPageRange()
{
	XubString out;
	int nFirst;
	int nLast;
	if ( NSPrintInfo_pageRange( getNativePrinterJob(), &nFirst, &nLast ) )
	{
		out = XubString::CreateFromInt32( nFirst );
		out += '-';
		out += XubString::CreateFromInt32( nLast );
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

sal_Bool com_sun_star_vcl_VCLPrintJob::startJob( com_sun_star_vcl_VCLPageFormat *_par0, ::rtl::OUString _par1 ) 
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(Lcom/sun/star/vcl/VCLPageFormat;Ljava/lang/String;)Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "startJob", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[2];
			args[0].l = _par0->getJavaObject();
			args[1].l = StringToJavaString( t.pEnv, _par1 );
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
