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

#define _SV_COM_SUN_STAR_VCL_VCLFONT_CXX

#ifndef _SV_COM_SUN_STAR_VCL_VCLFONT_HXX
#include <com/sun/star/vcl/VCLFont.hxx>
#endif

#ifdef MACOSX

#include <map>

#include <premac.h>
#include <Carbon/Carbon.h>
#include <postmac.h>

static ::std::map< ::rtl::OUString, sal_uInt32 > aNativeFontMap;

#endif	// MACOSX

using namespace rtl;
using namespace vcl;

// ============================================================================

#ifdef MACOSX
static sal_uInt32 GetMacFontFace( com_sun_star_vcl_VCLFont *pVCLFont )
{
	sal_uInt32 out = 0;

	OUString aFontName( pVCLFont->getName() );
	::std::map< OUString, sal_uInt32 >::iterator it = aNativeFontMap.find( aFontName );
	if ( it != aNativeFontMap.end() )
		out = it->second;

	if ( !out )
	{
		VCLThreadAttach t;
		if ( t.pEnv )
		{
			java_lang_Object *peer = pVCLFont->getPeer();
			if ( peer )
			{
				jobject tempObj = peer->getJavaObject();
				if ( tempObj )
				{
					// Test the JVM version and if it is below 1.4, use
					// Font Manager APIs
					if ( t.pEnv->GetVersion() < JNI_VERSION_1_4 )
					{
						jclass tempClass = t.pEnv->FindClass( "com/apple/mrj/internal/awt/graphics/VFontPeer" );
						if ( tempClass && t.pEnv->IsInstanceOf( tempObj, tempClass ) )
						{
							static jfieldID fIDMacFont = NULL;
							if ( !fIDMacFont )
							{
								char *cSignature = "S";
								fIDMacFont = t.pEnv->GetFieldID( tempClass, "fMacFont", cSignature );
							}
							OSL_ENSURE( fIDMacFont, "Unknown field id!" );
							if ( fIDMacFont )
							{
								FMFontFamily nFamily = (FMFontFamily)t.pEnv->GetShortField( tempObj, fIDMacFont );
								FMFontFamilyInstanceIterator aIterator;
								if ( FMCreateFontFamilyInstanceIterator( nFamily, &aIterator ) == noErr )
								{
									FMFont nFont;
									FMFontStyle nStyle;
									FMFontSize nSize;
									while ( FMGetNextFontFamilyInstance( &aIterator, &nFont, &nStyle, &nSize ) == noErr )
									{
										ATSFontRef aFont = FMGetATSFontRefFromFont( nFont );
										CFStringRef aFontNameRef;
										if ( aFont && ATSFontGetName( aFont, kATSOptionFlagsDefault, &aFontNameRef ) == noErr )
										{
											sal_Int32 nBufSize = CFStringGetLength( aFontNameRef );
											sal_Unicode aBuf[ nBufSize + 1 ];
											CFRange aRange;

											aRange.location = 0;
											aRange.length = nBufSize;
											CFStringGetCharacters( aFontNameRef, aRange, aBuf );
											aBuf[ nBufSize ] = 0;
											CFRelease( aFontNameRef );

											sal_uInt32 nFontFace = ( nFamily << 16 ) | nStyle;
											OUString aName( aBuf );
											aNativeFontMap[ aName ] = nFontFace;
											if ( aFontName == aName )
												out = nFontFace;
										}
									}
								}

								// Handle Java's font alias names
								if ( !out )
								{
									sal_uInt32 nFontFace = ( nFamily << 16 );
									aNativeFontMap[ aFontName ] = nFontFace;
									out = nFontFace;
								}
							}
						}
					}
				}
				delete peer;
			}
		}
	}

	return out;
}
#endif	// MACOSX

// ============================================================================

jclass com_sun_star_vcl_VCLFont::theClass = NULL;

// ----------------------------------------------------------------------------

jboolean com_sun_star_vcl_VCLFont::useDefaultFont = FALSE;

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

::std::map< OUString, com_sun_star_vcl_VCLFont* > com_sun_star_vcl_VCLFont::getAllFonts()
{
	::std::map< OUString, com_sun_star_vcl_VCLFont* > out;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		static jmethodID mID = NULL;
		if ( !mID )
		{
			char *cSignature = "()[Lcom/sun/star/vcl/VCLFont;";
			mID = t.pEnv->GetStaticMethodID( getMyClass(), "getAllFonts", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobjectArray tempArray;
			tempArray = (jobjectArray)t.pEnv->CallStaticObjectMethod( getMyClass(), mID );
			if ( tempArray )
			{
				jsize nSize = t.pEnv->GetArrayLength( tempArray );
				for ( jsize i = 0; i < nSize; i++ )
				{
					jobject tempObj = t.pEnv->GetObjectArrayElement( tempArray, i );
					if ( !tempObj )
						continue;

					com_sun_star_vcl_VCLFont *pVCLFont = new com_sun_star_vcl_VCLFont( tempObj );
#ifdef MACOSX
					if ( kATSUInvalidFontID == (ATSUFontID)pVCLFont->getNativeFont() )
					{
						delete pVCLFont;
						continue;
					}
#endif	// MACOSX
					OUString aFontName( pVCLFont->getName() );
					::std::map< OUString, com_sun_star_vcl_VCLFont* >::iterator it = out.find( aFontName );
					if ( it != out.end() )
						delete it->second;
					out[ aFontName ] = pVCLFont;
				}
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLFont *com_sun_star_vcl_VCLFont::deriveFont( long _par0, sal_Bool _par1, sal_Bool _par2, short _par3, sal_Bool _par4, sal_Bool _par5 )
{
	static jmethodID mID = NULL;
	com_sun_star_vcl_VCLFont *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "(IZZSZZ)Lcom/sun/star/vcl/VCLFont;";
			mID = t.pEnv->GetMethodID( getMyClass(), "deriveFont", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[6];
			args[0].i = jint( _par0 );
			args[1].z = jboolean( _par1 );
			args[2].z = jboolean( _par2 );
			args[3].s = jshort( _par3 );
			args[4].z = jboolean( _par4 );
			args[5].z = jboolean( _par5 );
			jobject tempObj = t.pEnv->CallNonvirtualObjectMethodA( object, getMyClass(), mID, args );
			if ( tempObj )
				out = new com_sun_star_vcl_VCLFont( tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

long com_sun_star_vcl_VCLFont::getAscent()
{
	static jmethodID mID = NULL;
	long out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getAscent", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			if ( com_sun_star_vcl_VCLFont::useDefaultFont )
			{
				com_sun_star_vcl_VCLFont *pDefaultFont = getDefaultFont();
				out = (long)t.pEnv->CallNonvirtualIntMethod( pDefaultFont->getJavaObject(), getMyClass(), mID );
				delete pDefaultFont;
			}
			else
			{
				out = (long)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLFont *com_sun_star_vcl_VCLFont::getDefaultFont()
{
	static jmethodID mID = NULL;
	com_sun_star_vcl_VCLFont *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Lcom/sun/star/vcl/VCLFont;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getDefaultFont", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj;
			tempObj = t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
				out = new com_sun_star_vcl_VCLFont( tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

long com_sun_star_vcl_VCLFont::getDescent()
{
	static jmethodID mID = NULL;
	long out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getDescent", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			if ( com_sun_star_vcl_VCLFont::useDefaultFont )
			{
				com_sun_star_vcl_VCLFont *pDefaultFont = getDefaultFont();
				out = (long)t.pEnv->CallNonvirtualIntMethod( pDefaultFont->getJavaObject(), getMyClass(), mID );
				delete pDefaultFont;
			}
			else
			{
				out = (long)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

FontFamily com_sun_star_vcl_VCLFont::getFamilyType()
{
	static jmethodID mID = NULL;
	FontFamily out = FAMILY_DONTKNOW;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getFamilyType", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (FontFamily)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

long com_sun_star_vcl_VCLFont::getKerning( USHORT _par0, USHORT _par1 )
{
	static jmethodID mID = NULL;
	long out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getKerning", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jvalue args[2];
			args[0].c = jchar( _par0 );
			args[1].c = jchar( _par1 );
			if ( com_sun_star_vcl_VCLFont::useDefaultFont )
			{
				com_sun_star_vcl_VCLFont *pDefaultFont = getDefaultFont();
				out = (long)t.pEnv->CallNonvirtualIntMethodA( pDefaultFont->getJavaObject(), getMyClass(), mID, args );
				delete pDefaultFont;
			}
			else
			{
				out = (long)t.pEnv->CallNonvirtualIntMethodA( object, getMyClass(), mID, args );
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

long com_sun_star_vcl_VCLFont::getLeading()
{
	static jmethodID mID = NULL;
	long out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getLeading", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			if ( com_sun_star_vcl_VCLFont::useDefaultFont )
			{
				com_sun_star_vcl_VCLFont *pDefaultFont = getDefaultFont();
				out = (long)t.pEnv->CallNonvirtualIntMethod( pDefaultFont->getJavaObject(), getMyClass(), mID );
				delete pDefaultFont;
			}
			else
			{
				out = (long)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
			}
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

OUString com_sun_star_vcl_VCLFont::getName()
{
	static jmethodID mID = NULL;
	OUString out;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/lang/String;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getName", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jstring tempObj;
			tempObj = (jstring)t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			if ( tempObj )
				out = JavaString2String( t.pEnv, tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

void *com_sun_star_vcl_VCLFont::getNativeFont()
{
	void *out = NULL;

#ifdef MACOSX
	sal_uInt32 nFontFace;
	if ( com_sun_star_vcl_VCLFont::useDefaultFont )
	{
		com_sun_star_vcl_VCLFont *pDefaultFont = getDefaultFont();
		nFontFace = GetMacFontFace( pDefaultFont );
		delete pDefaultFont;
	}
	else
	{
		nFontFace = GetMacFontFace( this );
	}

	if ( nFontFace )
	{
		FMFontFamily nFont = (FMFontFamily)( nFontFace >> 16 );
		FMFontStyle nStyle = (FMFontStyle)( nFontFace & 0x00ff );
		FMFont aFont;
		if ( FMGetFontFromFontFamilyInstance( nFont, nStyle, &aFont, NULL ) == noErr )
			out = (void *)FMGetATSFontRefFromFont( aFont );
	}
#endif	// MACOSX

	return out;
}

// ----------------------------------------------------------------------------

void *com_sun_star_vcl_VCLFont::getNativeFont( sal_Bool _par0, sal_Bool _par1 )
{
	void *out = NULL;

#ifdef MACOSX
	sal_uInt32 nFontFace;
	if ( com_sun_star_vcl_VCLFont::useDefaultFont )
	{
		com_sun_star_vcl_VCLFont *pDefaultFont = getDefaultFont();
		nFontFace = GetMacFontFace( pDefaultFont );
		delete pDefaultFont;
	}
	else
	{
		nFontFace = GetMacFontFace( this );
	}

	if ( nFontFace )
	{
		FMFontFamily nFont = (FMFontFamily)( nFontFace >> 16 );
		FMFontStyle nStyle = (FMFontStyle)( nFontFace & 0x00ff );
		if ( !_par0 && !_par1 )
		{
			nStyle &= ~( bold | italic );
		}
		else
		{
			if ( _par0 )
				nStyle |= bold;
			if ( _par1 )
				nStyle |= italic;
		}

		FMFont aFont;
		if ( FMGetFontFromFontFamilyInstance( nFont, nStyle, &aFont, NULL ) == noErr )
			out = (void *)FMGetATSFontRefFromFont( aFont );
	}
#endif	// MACOSX

	return out;
}

// ----------------------------------------------------------------------------

short com_sun_star_vcl_VCLFont::getOrientation()
{
	static jmethodID mID = NULL;
	short out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()S";
			mID = t.pEnv->GetMethodID( getMyClass(), "getOrientation", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (short)t.pEnv->CallNonvirtualShortMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

java_lang_Object *com_sun_star_vcl_VCLFont::getPeer()
{
	static jmethodID mID = NULL;
	java_lang_Object *out = NULL;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Ljava/awt/peer/FontPeer;";
			mID = t.pEnv->GetMethodID( getMyClass(), "getPeer", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
		{
			jobject tempObj;
			if ( com_sun_star_vcl_VCLFont::useDefaultFont )
			{
				com_sun_star_vcl_VCLFont *pDefaultFont = getDefaultFont();
				tempObj = t.pEnv->CallNonvirtualObjectMethod( pDefaultFont->getJavaObject(), getMyClass(), mID );
				delete pDefaultFont;
			}
			else
			{
				tempObj = t.pEnv->CallNonvirtualObjectMethod( object, getMyClass(), mID );
			}
			if ( tempObj )
				out = new java_lang_Object( tempObj );
		}
	}
	return out;
}

// ----------------------------------------------------------------------------

long com_sun_star_vcl_VCLFont::getSize()
{
	static jmethodID mID = NULL;
	long out = 0;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()I";
			mID = t.pEnv->GetMethodID( getMyClass(), "getSize", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (long)t.pEnv->CallNonvirtualIntMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLFont::isAntialiased()
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "isAntialiased", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (sal_Bool)t.pEnv->CallNonvirtualBooleanMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLFont::isBold()
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "isBold", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (sal_Bool)t.pEnv->CallNonvirtualBooleanMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLFont::isItalic()
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "isItalic", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (sal_Bool)t.pEnv->CallNonvirtualBooleanMethod( object, getMyClass(), mID );
	}
	return out;
}

// ----------------------------------------------------------------------------

sal_Bool com_sun_star_vcl_VCLFont::isVertical()
{
	static jmethodID mID = NULL;
	sal_Bool out = sal_False;
	VCLThreadAttach t;
	if ( t.pEnv )
	{
		if ( !mID )
		{
			char *cSignature = "()Z";
			mID = t.pEnv->GetMethodID( getMyClass(), "isVertical", cSignature );
		}
		OSL_ENSURE( mID, "Unknown method id!" );
		if ( mID )
			out = (sal_Bool)t.pEnv->CallNonvirtualBooleanMethod( object, getMyClass(), mID );
	}
	return out;
}
