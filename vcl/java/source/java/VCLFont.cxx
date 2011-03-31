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
#ifndef _SV_SALATSLAYOUT_HXX
#include <salatslayout.hxx>
#endif
#ifndef _SV_SALGDI_H
#include <salgdi.h>
#endif
#ifndef _SV_SALDATA_HXX
#include <saldata.hxx>
#endif

using namespace rtl;
using namespace vcl;

// ============================================================================

jclass com_sun_star_vcl_VCLFont::theClass = NULL;

// ----------------------------------------------------------------------------

::std::map< com_sun_star_vcl_VCLFont*, com_sun_star_vcl_VCLFont* >com_sun_star_vcl_VCLFont::maInstancesMap;

// ----------------------------------------------------------------------------

void com_sun_star_vcl_VCLFont::clearNativeFonts()
{
#ifdef USE_CORETEXT_TEXT_RENDERING
	for ( ::std::map< com_sun_star_vcl_VCLFont*, com_sun_star_vcl_VCLFont* >::const_iterator vfit = com_sun_star_vcl_VCLFont::maInstancesMap.begin(); vfit != com_sun_star_vcl_VCLFont::maInstancesMap.end(); ++vfit )
	{
		if ( vfit->second->mnNativeFont )
		{
			CFRelease( (CTFontRef)vfit->second->mnNativeFont );
			vfit->second->mnNativeFont = 0;
		}
	}

	GetSalData()->maJavaNativeFontMapping.clear();
#endif	// USE_CORETEXT_TEXT_RENDERING
}

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

com_sun_star_vcl_VCLFont::com_sun_star_vcl_VCLFont( OUString aName, float fSize, short nOrientation, sal_Bool bAntialiased, sal_Bool bVertical, double fScaleX ) : java_lang_Object( (jobject)NULL ), maName( aName ), mnNativeFont( 0 ), mnOrientation( nOrientation ), mfScaleX( fScaleX ), mfSize( fSize ), mbAntialiased( bAntialiased ), mbVertical( bVertical )
{
#ifdef USE_CORETEXT_TEXT_RENDERING
	com_sun_star_vcl_VCLFont::maInstancesMap[ this ] = this;
#endif	// USE_CORETEXT_TEXT_RENDERING

	static jmethodID mID = NULL;
	VCLThreadAttach t;
	if ( !t.pEnv )
		return;
	if ( !mID )
	{
		char *cSignature = "(Ljava/lang/String;FZD)V";
		mID = t.pEnv->GetMethodID( getMyClass(), "<init>", cSignature );
	}
	OSL_ENSURE( mID, "Unknown method id!" );

	jvalue args[4];
	args[0].l = StringToJavaString( t.pEnv, maName );
	args[1].f = jfloat( mfSize );
	args[2].z = jboolean( mbAntialiased );
	args[3].d = jdouble( mfScaleX );
	jobject tempObj;
	tempObj = t.pEnv->NewObjectA( getMyClass(), mID, args );
	saveRef( tempObj );
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLFont::com_sun_star_vcl_VCLFont( com_sun_star_vcl_VCLFont *pVCLFont ) : java_lang_Object( (jobject)pVCLFont->getJavaObject() ), maName( pVCLFont->maName ), mnNativeFont( pVCLFont->mnNativeFont ), mnOrientation( pVCLFont->mnOrientation ), mfScaleX( pVCLFont->mfScaleX ), mfSize( pVCLFont->mfSize ), mbAntialiased( pVCLFont->mbAntialiased ), mbVertical( pVCLFont->mbVertical )
{
#ifdef USE_CORETEXT_TEXT_RENDERING
	if ( mnNativeFont )
		CFRetain( (CTFontRef)mnNativeFont );

	com_sun_star_vcl_VCLFont::maInstancesMap[ this ] = this;
#endif	// USE_CORETEXT_TEXT_RENDERING
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLFont::~com_sun_star_vcl_VCLFont()
{
#ifdef USE_CORETEXT_TEXT_RENDERING
	::std::map< com_sun_star_vcl_VCLFont*, com_sun_star_vcl_VCLFont* >::iterator it = com_sun_star_vcl_VCLFont::maInstancesMap.find( this );
	if ( it != com_sun_star_vcl_VCLFont::maInstancesMap.end() )
		com_sun_star_vcl_VCLFont::maInstancesMap.erase( it );

	if ( mnNativeFont )
		CFRelease( (CTFontRef)mnNativeFont );
#endif	// USE_CORETEXT_TEXT_RENDERING
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
			if ( jit != pSalData->maJavaFontNameMapping.end() && jit->second->mnNativeFontID )
			{
				mnNativeFont = jit->second->mnNativeFontID;
				pSalData->maJavaNativeFontMapping[ aPSName ] = mnNativeFont;
			}
			else
			{
				// Fix bug 1611 by adding another search for mismatched names
				CFStringRef aString = CFStringCreateWithCharactersNoCopy( NULL, aPSName.getStr(), aPSName.getLength(), kCFAllocatorNull );
				if ( aString )
				{
#ifdef USE_CORETEXT_TEXT_RENDERING
					CTFontRef aFont = CTFontCreateWithName( aString, 0, NULL );
					if ( aFont )
					{
						mnNativeFont = (sal_IntPtr)aFont;
						pSalData->maJavaNativeFontMapping[ aPSName ] = mnNativeFont;
					}
#else	// USE_CORETEXT_TEXT_RENDERING
					ATSFontRef aFont = ATSFontFindFromPostScriptName( aString, kATSOptionFlagsDefault );
					if ( aFont )
					{
						mnNativeFont = (int)SalATSLayout::GetNativeFontFromATSFontRef( aFont );
						pSalData->maJavaNativeFontMapping[ aPSName ] = mnNativeFont;
					}
#endif	// USE_CORETEXT_TEXT_RENDERING

					CFRelease( aString );
				}
			}
		}
		else
		{
			mnNativeFont = it->second;
		}

#ifdef USE_CORETEXT_TEXT_RENDERING
		// Fix bug 3653 by always retaining any native font as even when
		// CTFontCreateWithName() is called, the returned font will be
		// released if Mac OS X removes or disables the underlying font
		if ( mnNativeFont )
			CFRetain( (CTFontRef)mnNativeFont );
#endif	// USE_CORETEXT_TEXT_RENDERING
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

float com_sun_star_vcl_VCLFont::getSize()
{
	return mfSize;
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
