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

#ifdef MACOSX
#include <math.h>
#endif

using namespace vcl;

// ============================================================================

jclass com_sun_star_vcl_VCLPrintJob::theClass = NULL;

// ----------------------------------------------------------------------------
  
const Rectangle com_sun_star_vcl_VCLPrintJob::getImageableBounds()
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
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "getImageableBounds", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallStaticObjectMethod( getMyClass(), mID );
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

Orientation com_sun_star_vcl_VCLPrintJob::getOrientation()
{
	static jmethodID mID = NULL;
	Orientation out = ORIENTATION_PORTRAIT;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "getOrientation", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (Orientation)t.pEnv->CallStaticBooleanMethod( getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------
  
const Size com_sun_star_vcl_VCLPrintJob::getPageSize()
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
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "getPageSize", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallStaticObjectMethod( getMyClass(), mID );
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

void com_sun_star_vcl_VCLPrintJob::setup()
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()V";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "setup", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			t.pEnv->CallStaticVoidMethod( getMyClass(), mID );
	}
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

#ifdef MACOSX
	// Mac OS X does not update the JobAttributes that we pass to the print
	// job so we need to update it ourselves
	static jfieldID fIDPrintJob = NULL;
	static jfieldID fIDPrinterJob = NULL;
	static jmethodID mIDGetFirstPage = NULL;
	static jmethodID mIDGetLastPage = NULL;
	static jfieldID fIDPageRanges = NULL;
	jclass tempObjClass = t.pEnv->GetObjectClass( tempObj );
	if ( !fIDPrintJob )
	{
		char *cSignature = "Ljava/awt/PrintJob;";
		fIDPrintJob = t.pEnv->GetFieldID( tempObjClass, "printJob", cSignature );
	}
	OSL_ENSURE( fIDPrintJob, "Unknown field id!" );
	if ( fIDPrintJob )
	{
		jobject printJob = t.pEnv->GetObjectField( tempObj, fIDPrintJob );
		if ( printJob )
		{
			jclass printJobClass = t.pEnv->GetObjectClass( printJob );
			if ( !fIDPrinterJob )
			{
				char *cSignature = "Ljava/awt/print/PrinterJob;";
				fIDPrinterJob = t.pEnv->GetFieldID( printJobClass, "printerJob", cSignature );
			}
			OSL_ENSURE( fIDPrinterJob, "Unknown field id!" );
			if ( fIDPrinterJob )
			{
				jobject printerJob = t.pEnv->GetObjectField( printJob, fIDPrinterJob );
				if ( printerJob )
				{
					jclass printerJobClass = t.pEnv->GetObjectClass( printerJob );
					if ( !mIDGetFirstPage )
					{
						char *cSignature = "()I";
						mIDGetFirstPage = t.pEnv->GetMethodID( printerJobClass, "getFirstPage", cSignature );
					}
					OSL_ENSURE( mIDGetFirstPage, "Unknown method id!" );
					if ( !mIDGetLastPage )
					{
						char *cSignature = "()I";
						mIDGetLastPage = t.pEnv->GetMethodID( printerJobClass, "getLastPage", cSignature );
					}
					OSL_ENSURE( mIDGetLastPage, "Unknown method id!" );
					if ( mIDGetFirstPage && mIDGetLastPage )
					{
						jint pageNumbers[2];
						pageNumbers[0] = t.pEnv->CallIntMethod( printerJob, mIDGetFirstPage );
						if ( pageNumbers[0] < pow( 2, 31 ) )
							pageNumbers[0]++;
						pageNumbers[1] = t.pEnv->CallIntMethod( printerJob, mIDGetLastPage );
						if ( pageNumbers[1] < pow( 2, 31 ) )
							pageNumbers[1]++;
						if ( !fIDPageRanges )
						{
							char *cSignature = "[[I";
							fIDPageRanges = t.pEnv->GetFieldID( tempObjClass, "pageRanges", cSignature );
						}
						OSL_ENSURE( fIDPageRanges, "Unknown field id!" );
						if ( fIDPageRanges )
						{
							jintArray pages = t.pEnv->NewIntArray( 2 );
							t.pEnv->SetIntArrayRegion( pages, 0, 2, pageNumbers );
							jobjectArray pageRanges = t.pEnv->NewObjectArray( 1, t.pEnv->GetObjectClass( pages ), NULL );
							t.pEnv->SetObjectArrayElement( pageRanges, 0, pages );
							t.pEnv->SetObjectField( tempObj, fIDPageRanges, pageRanges );
						}
					}
				}
			}
		}
	}
#endif	// MACOSX

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
			t.pEnv->CallVoidMethod( object, mID );
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
			t.pEnv->CallVoidMethod( object, mID );
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
			t.pEnv->CallVoidMethod( object, mID );
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
			t.pEnv->CallVoidMethod( object, mID );
	}
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLPrintJob::startJob()
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "startJob", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (sal_Bool)t.pEnv->CallBooleanMethod( object, mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLGraphics *com_sun_star_vcl_VCLPrintJob::startPage()
{
	static jmethodID mID = NULL;
	com_sun_star_vcl_VCLGraphics *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Lcom/sun/star/vcl/VCLGraphics;";
			mID = t.pEnv->GetMethodID( getMyClass(), "startPage", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallObjectMethod( object, mID );
			if ( tempObj )
				out = new com_sun_star_vcl_VCLGraphics( tempObj );
		}
	}
	return out;
}
