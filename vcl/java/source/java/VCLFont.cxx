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

#define _SV_COM_SUN_STAR_VCL_VCLFONT_CXX

#ifndef _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
#include <com/sun/star/vcl/VCLFont.hxx>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

using namespace rtl;
using namespace vcl;

// ============================================================================

jclass com_sun_star_vcl_VCLFont::theClass = NULL;

// ----------------------------------------------------------------------------

jclass com_sun_star_vcl_VCLFont::getMyClass()
{
	if ( !theClass )
	{
		VCLThreadAttach t;
		if ( !t.pEnv ) return (jclass)NULL;
		jclass tempClass = t.pEnv->FindClass( "com/sun/star/vcl/VCLFont" );
		OSL_ENSURE( tempClass, "Java : FindClass not found!" );
		theClass = (jclass)t.pEnv->NewGlobalRef( tempClass );
	}
	return theClass;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLFont::com_sun_star_vcl_VCLFont( OUString aName, long nSize, short nOrientation, sal_Bool bAntialiased, sal_Bool bVertical, double fScaleX ) : java_lang_Object( (jobject)NULL ), maName( aName ), mnNativeFont( 0 ), mnOrientation( nOrientation ), mfScaleX( fScaleX ), mnSize( nSize ), mbAntialiased( bAntialiased ), mbVertical( bVertical )
{
	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( !t.pEnv )
		return;
	if ( !mID )
	{
		char *cSignature = "(Ljava/lang/String;IZD)V";
		mID = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );
	}
	OSL_ENSURE( mID, "Unknown method id!" );

	jvalue args[4];
	args[0].l = StringToJavaString( t.pEnv, maName );
	args[1].i = jint( mnSize );
	args[2].z = jboolean( mbAntialiased );
	args[3].d = jdouble( mfScaleX );
	jobject tempObj;
	tempObj = t.pEnv->NewObjectA( getMyClass(), mID, args );
	saveRef( tempObj );
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLFont::com_sun_star_vcl_VCLFont( com_sun_star_vcl_VCLFont *pVCLFont ) : java_lang_Object( (jobject)pVCLFont->getJavaObject() ), maName( pVCLFont->maName ), mnNativeFont( pVCLFont->mnNativeFont ), mnOrientation( pVCLFont->mnOrientation ), mfScaleX( pVCLFont->mfScaleX ), mnSize( pVCLFont->mnSize ), mbAntialiased( pVCLFont->mbAntialiased ), mbVertical( pVCLFont->mbVertical )
{
}

// ----------------------------------------------------------------------------

OUString com_sun_star_vcl_VCLFont::getName()
{
	return maName;
}

// ----------------------------------------------------------------------------

sal_IntPtr com_sun_star_vcl_VCLFont::getNativeFont()
{
	if ( !mnNativeFont )
	{
		SalData *pSalData = GetSalData();

		OUString aPSName( getPSName() );
		::std::hash_map< OUString, sal_IntPtr, OUStringHash >::iterator it = pSalData->maJavaNativeFontMapping.find( aPSName );
		if ( it == pSalData->maJavaNativeFontMapping.end() )
		{
			::std::hash_map< OUString, JavaImplFontData*, OUStringHash >::iterator jit = pSalData->maJavaFontNameMapping.find( aPSName );
			if ( jit != pSalData->maJavaFontNameMapping.end() )
			{
				mnNativeFont = jit->second->mnATSUFontID;
				pSalData->maJavaNativeFontMapping[ aPSName ] = mnNativeFont;
			}
			else
			{
				// Fix bug 1611 by adding another search for mismatched names
				CFStringRef aString = CFStringCreateWithCharactersNoCopy( NULL, aPSName.getStr(), aPSName.getLength(), kCFAllocatorNull );
				if ( aString )
				{
					ATSFontRef aFont = ATSFontFindFromPostScriptName( aString, kATSOptionFlagsDefault );
					if ( aFont )
					{
						mnNativeFont = (int)FMGetFontFromATSFontRef( aFont );
						pSalData->maJavaNativeFontMapping[ aPSName ] = mnNativeFont;
					}

					CFRelease( aString );
				}
			}
		}
		else
		{
			mnNativeFont = it->second;
		}
	}

	return mnNativeFont;
}

// ----------------------------------------------------------------------------

short com_sun_star_vcl_VCLFont::getOrientation()
{
	return mnOrientation;
}

// ----------------------------------------------------------------------------

OUString com_sun_star_vcl_VCLFont::getPSName()
{
	if ( !maPSName.getLength() )
	{
		static jmethodID mID = NULL;
		VCLThreadAttach t;
		if ( t.pEnv )
		{
			if ( !mID )
			{
				char *cSignature = "()Ljava/lang/String;";
				mID = t.pEnv->GetMethodID( getMyClass(), "getPSName", cSignature );
			}
			OSL_ENSURE( mID, "Unknown method id!" );
			if ( mID )
			{
				jstring tempObj;
				tempObj = (jstring)t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
				if ( tempObj )
					maPSName = JavaString2String( t.pEnv, tempObj );
			}
		}
	}

	return maPSName;
}

// ----------------------------------------------------------------------------

double com_sun_star_vcl_VCLFont::getScaleX()
{
	return mfScaleX;
}

// ----------------------------------------------------------------------------

long com_sun_star_vcl_VCLFont::getSize()
{
	return mnSize;
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLFont::isAntialiased()
{
	return mbAntialiased;
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLFont::isVertical()
{
	return mbVertical;
}
