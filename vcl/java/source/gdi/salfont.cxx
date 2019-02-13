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

#include "java/salatslayout.hxx"
#include "java/saldata.hxx"
#include "java/salgdi.h"

// ============================================================================

void JavaImplFont::clearNativeFonts()
{
	GetSalData()->maJavaNativeFontMapping.clear();
}

// ----------------------------------------------------------------------------

JavaImplFont::JavaImplFont( OUString aName, float fSize, short nOrientation, sal_Bool bAntialiased, sal_Bool bVertical, double fScaleX ) : maPSName( aName ), mnNativeFont( 0 ), mnOrientation( nOrientation ), mfScaleX( fScaleX ), mfSize( fSize ), mbAntialiased( bAntialiased ), mbVertical( bVertical ), mbNativeFontOwner( sal_True )
{
}

// ----------------------------------------------------------------------------

JavaImplFont::JavaImplFont( JavaImplFont *pFont ) : maPSName( pFont->maPSName ), mnNativeFont( pFont->mnNativeFont ), mnOrientation( pFont->mnOrientation ), mfScaleX( pFont->mfScaleX ), mfSize( pFont->mfSize ), mbAntialiased( pFont->mbAntialiased ), mbVertical( pFont->mbVertical ), mbNativeFontOwner( sal_True )
{
	if ( mnNativeFont )
		CFRetain( reinterpret_cast< CTFontRef >( mnNativeFont ) );
}

// ----------------------------------------------------------------------------

JavaImplFont::~JavaImplFont()
{
	if ( mnNativeFont && mbNativeFontOwner )
		CFRelease( reinterpret_cast< CTFontRef >( mnNativeFont ) );
}

// ----------------------------------------------------------------------------

sal_IntPtr JavaImplFont::getNativeFont()
{
	if ( !mnNativeFont )
	{
		SalData *pSalData = GetSalData();

		OUString aPSName( getPSName() );
		::std::unordered_map< OUString, sal_IntPtr, OUStringHash >::iterator it = pSalData->maJavaNativeFontMapping.find( aPSName );
		if ( it == pSalData->maJavaNativeFontMapping.end() )
		{
			::std::unordered_map< OUString, JavaPhysicalFontFace*, OUStringHash >::iterator jit = pSalData->maJavaFontNameMapping.find( aPSName );
			if ( jit != pSalData->maJavaFontNameMapping.end() && jit->second->mnNativeFontID )
			{
				mnNativeFont = jit->second->mnNativeFontID;
				pSalData->maJavaNativeFontMapping[ aPSName ] = mnNativeFont;
			}
			else
			{
				// Fix bug 1611 by adding another search for mismatched names
				CFStringRef aString = CFStringCreateWithCharactersNoCopy( nullptr, reinterpret_cast< const UniChar* >( aPSName.getStr() ), aPSName.getLength(), kCFAllocatorNull );
				if ( aString )
				{
					CTFontRef aFont = CTFontCreateWithName( aString, 0, nullptr );
					if ( aFont )
					{
						// Fix bug 3653 by never releasing this font as this
						// is a font loaded internally by Java and Java will
						// release the font out from underneath us
						mbNativeFontOwner = sal_False;
						mnNativeFont = reinterpret_cast< sal_IntPtr >( aFont );
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

		// Fix bug 3653 by always retaining any native font as even when
		// CTFontCreateWithName() is called, the returned font will be
		// released if Mac OS X removes or disables the underlying font
		if ( mnNativeFont )
			CFRetain( reinterpret_cast< CTFontRef >( mnNativeFont ) );
	}

	return mnNativeFont;
}

// ----------------------------------------------------------------------------

short JavaImplFont::getOrientation()
{
	return mnOrientation;
}

// ----------------------------------------------------------------------------

OUString JavaImplFont::getPSName()
{
	return maPSName;
}

// ----------------------------------------------------------------------------

double JavaImplFont::getScaleX()
{
	return mfScaleX;
}

// ----------------------------------------------------------------------------

float JavaImplFont::getSize()
{
	return mfSize;
}

// ----------------------------------------------------------------------------

sal_Bool JavaImplFont::isAntialiased()
{
	return mbAntialiased;
}

// ----------------------------------------------------------------------------

sal_Bool JavaImplFont::isVertical()
{
	return mbVertical;
}
