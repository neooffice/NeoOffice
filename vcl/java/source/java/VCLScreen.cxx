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

#define _SV_COM_SUN_STAR_VCL_VCLSCREEN_CXX

#ifndef _SV_COM_SUN_STAR_VCL_VCLSCREEN_HXX
#include <com/sun/star/vcl/VCLScreen.hxx>
#endif

using namespace vcl;

// ============================================================================

jclass com_sun_star_vcl_VCLScreen::theClass = NULL;

// ----------------------------------------------------------------------------

jclass com_sun_star_vcl_VCLScreen::getMyClass()
{
	if ( !theClass )
	{
		VCLThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;
		jclass tempClass = t.pEnv->FindClass( "com/sun/star/vcl/VCLScreen" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

const Rectangle com_sun_star_vcl_VCLScreen::getFrameInsets()
{
	static jmethodID mID = NULL;
	static jfieldID fIDLeft = NULL;
	static jfieldID fIDTop = NULL;
	static jfieldID fIDRight = NULL;
	static jfieldID fIDBottom = NULL;
	Rectangle out( 0, 0, 0, 0 );
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/awt/Insets;";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "getFrameInsets", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallStaticObjectMethod( getMyClass(), mID );
			if ( tempObj )
			{
				jclass tempObjClass = t.pEnv->GetObjectClass( tempObj );
				if ( !fIDLeft )
				{
					char *cSignature = "I";
					fIDLeft = t.pEnv->GetFieldID( tempObjClass, "left", cSignature );
				}
				OSL_ENSURE( fIDLeft, "Unknown field id!" );
				if ( !fIDTop )
				{
					char *cSignature = "I";
					fIDTop  = t.pEnv->GetFieldID( tempObjClass, "top", cSignature );
				}
				OSL_ENSURE( fIDTop, "Unknown field id!" );
				if ( !fIDRight )
				{
					char *cSignature = "I";
					fIDRight = t.pEnv->GetFieldID( tempObjClass, "right", cSignature );
				}
				OSL_ENSURE( fIDRight, "Unknown field id!" );
				if ( !fIDBottom )
				{
					char *cSignature = "I";
					fIDBottom = t.pEnv->GetFieldID( tempObjClass, "bottom", cSignature );
				}
				OSL_ENSURE( fIDBottom, "Unknown field id!" );
				if ( fIDLeft && fIDTop && fIDRight && fIDBottom )
				{
					out = Rectangle( (long)t.pEnv->GetIntField( tempObj, fIDLeft ), (long)t.pEnv->GetIntField( tempObj, fIDTop ), (long)t.pEnv->GetIntField( tempObj, fIDRight ), (long)t.pEnv->GetIntField( tempObj, fIDBottom ) );
				}
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

const Size com_sun_star_vcl_VCLScreen::getScreenSize()
{
	static jmethodID mID = NULL;
	static jfieldID fIDWidth = NULL;
	static jfieldID fIDHeight = NULL;
	Size out;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/awt/Dimension;";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "getScreenSize", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj = t.pEnv->CallStaticObjectMethod( getMyClass(), mID );
			if ( tempObj )
			{
				jclass tempObjClass = t.pEnv->GetObjectClass( tempObj );
				OSL_ENSURE( tempObjClass, "Java : FindClass not found!" );
				if ( !fIDWidth )
				{
					char *cSignature = "I";
					fIDWidth = t.pEnv->GetFieldID( tempObjClass, "width", cSignature );
				}
				out.setWidth( (long)t.pEnv->GetIntField( tempObj, fIDWidth ) );
				if ( !fIDHeight )
				{
					char *cSignature = "I";
					fIDHeight = t.pEnv->GetFieldID( tempObjClass, "height", cSignature );
				}
				out.setHeight( (long)t.pEnv->GetIntField( tempObj, fIDHeight ) );
			}
		}
	}
	return out;
}
