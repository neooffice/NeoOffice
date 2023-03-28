/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
		CFRetain( (CTFontRef)mnNativeFont );
}

// ----------------------------------------------------------------------------

JavaImplFont::~JavaImplFont()
{
	if ( mnNativeFont && mbNativeFontOwner )
		CFRelease( (CTFontRef)mnNativeFont );
}

// ----------------------------------------------------------------------------

sal_IntPtr JavaImplFont::getNativeFont()
{
	if ( !mnNativeFont )
	{
		SalData *pSalData = GetSalData();

		OUString aPSName( getPSName() );
		::boost::unordered_map< OUString, sal_IntPtr, OUStringHash >::iterator it = pSalData->maJavaNativeFontMapping.find( aPSName );
		if ( it == pSalData->maJavaNativeFontMapping.end() )
		{
			::boost::unordered_map< OUString, JavaPhysicalFontFace*, OUStringHash >::iterator jit = pSalData->maJavaFontNameMapping.find( aPSName );
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
			CFRetain( (CTFontRef)mnNativeFont );
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
