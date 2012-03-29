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
 *  Patrick Luby, March 2012
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2012 Planamesa Inc.
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

#include <salgdi.h>
#include <salatslayout.hxx>
#include <saldata.hxx>

#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
#include <com/sun/star/vcl/VCLFont.hxx>
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING

using namespace rtl;
#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
using namespace vcl;
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING

// ============================================================================

::std::map< JavaImplFont*, JavaImplFont* > JavaImplFont::maInstancesMap;

// ----------------------------------------------------------------------------

void JavaImplFont::clearNativeFonts()
{
#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	com_sun_star_vcl_VCLFont::clearNativeFonts();
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
#ifdef USE_CORETEXT_TEXT_RENDERING
	for ( ::std::map< JavaImplFont*, JavaImplFont* >::const_iterator vfit = JavaImplFont::maInstancesMap.begin(); vfit != JavaImplFont::maInstancesMap.end(); ++vfit )
	{
		if ( vfit->second->mnNativeFont )
		{
			if ( vfit->second->mbNativeFontOwner )
				CFRelease( (CTFontRef)vfit->second->mnNativeFont );
			vfit->second->mnNativeFont = 0;
		}
	}

	GetSalData()->maJavaNativeFontMapping.clear();
#endif	// USE_CORETEXT_TEXT_RENDERING
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
}

// ----------------------------------------------------------------------------

#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
JavaImplFont::JavaImplFont( OUString aName, float fSize, short nOrientation, sal_Bool bAntialiased, sal_Bool bVertical, double fScaleX ) : mpVCLFont( NULL )
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
JavaImplFont::JavaImplFont( OUString aName, float fSize, short nOrientation, sal_Bool bAntialiased, sal_Bool bVertical, double fScaleX ) : maPSName( aName ), mnNativeFont( 0 ), mnOrientation( nOrientation ), mfScaleX( fScaleX ), mfSize( fSize ), mbAntialiased( bAntialiased ), mbVertical( bVertical ), mbNativeFontOwner( sal_True )
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
{
#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	mpVCLFont = new com_sun_star_vcl_VCLFont( aName, fSize, nOrientation, bAntialiased, bVertical, fScaleX );
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
#ifdef USE_CORETEXT_TEXT_RENDERING
	JavaImplFont::maInstancesMap[ this ] = this;
#endif	// USE_CORETEXT_TEXT_RENDERING
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
}

// ----------------------------------------------------------------------------

#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
JavaImplFont::JavaImplFont( JavaImplFont *pFont ) : mpVCLFont( NULL )
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
JavaImplFont::JavaImplFont( JavaImplFont *pFont ) : maPSName( pFont->maPSName ), mnNativeFont( pFont->mnNativeFont ), mnOrientation( pFont->mnOrientation ), mfScaleX( pFont->mfScaleX ), mfSize( pFont->mfSize ), mbAntialiased( pFont->mbAntialiased ), mbVertical( pFont->mbVertical ), mbNativeFontOwner( sal_True )
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
{
#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	mpVCLFont = new com_sun_star_vcl_VCLFont( pFont->getVCLFont() );
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
#ifdef USE_CORETEXT_TEXT_RENDERING
	if ( mnNativeFont )
		CFRetain( (CTFontRef)mnNativeFont );

	JavaImplFont::maInstancesMap[ this ] = this;
#endif	// USE_CORETEXT_TEXT_RENDERING
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
}

// ----------------------------------------------------------------------------

JavaImplFont::~JavaImplFont()
{
#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	if ( mpVCLFont )
		delete mpVCLFont;
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
#ifdef USE_CORETEXT_TEXT_RENDERING
	::std::map< JavaImplFont*, JavaImplFont* >::iterator it = JavaImplFont::maInstancesMap.find( this );
	if ( it != JavaImplFont::maInstancesMap.end() )
		JavaImplFont::maInstancesMap.erase( it );

	if ( mnNativeFont && mbNativeFontOwner )
		CFRelease( (CTFontRef)mnNativeFont );
#endif	// USE_CORETEXT_TEXT_RENDERING
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
}

// ----------------------------------------------------------------------------

sal_IntPtr JavaImplFont::getNativeFont()
{
#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	return mpVCLFont->getNativeFont();
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
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
						// Fix bug 3653 by never releasing this font as this
						// is a font loaded internally by Java and Java will
						// release the font out from underneath us
						mbNativeFontOwner = sal_False;
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
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
}

// ----------------------------------------------------------------------------

short JavaImplFont::getOrientation()
{
#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	return mpVCLFont->getOrientation();
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
	return mnOrientation;
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
}

// ----------------------------------------------------------------------------

OUString JavaImplFont::getPSName()
{
#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	return mpVCLFont->getPSName();
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
	return maPSName;
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
}

// ----------------------------------------------------------------------------

double JavaImplFont::getScaleX()
{
#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	return mpVCLFont->getScaleX();
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
	return mfScaleX;
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
}

// ----------------------------------------------------------------------------

float JavaImplFont::getSize()
{
#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	return mpVCLFont->getSize();
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
	return mfSize;
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
}

#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING

// ----------------------------------------------------------------------------

com_sun_star_vcl_VCLFont *JavaImplFont::getVCLFont()
{
	return mpVCLFont;
}

#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING

// ----------------------------------------------------------------------------

sal_Bool JavaImplFont::isAntialiased()
{
#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	return mpVCLFont->isAntialiased();
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
	return mbAntialiased;
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
}

// ----------------------------------------------------------------------------

sal_Bool JavaImplFont::isVertical()
{
#if !defined USE_NATIVE_WINDOW || !defined USE_NATIVE_VIRTUAL_DEVICE || !defined USE_NATIVE_PRINTING
	return mpVCLFont->isVertical();
#else	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
	return mbVertical;
#endif	// !USE_NATIVE_WINDOW || !USE_NATIVE_VIRTUAL_DEVICE || !USE_NATIVE_PRINTING
}
