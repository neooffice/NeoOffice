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
 *  Patrick Luby, July 2005
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2005 Planamesa Inc.
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

#include <tools/color.hxx>
#include <tools/gen.hxx>
#include <tools/stream.hxx>
#include <vcl/fntstyle.hxx>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>

#include "salgdi3_cocoa.h"

NSFont *NSFont_findPlainFont( NSFont *pNSFont )
{
	NSFont *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager )
		{
			// Find matching unbolded, unitalicized, medium weight font
			NSFont *pNewNSFont = [pFontManager fontWithFamily:[pNSFont familyName] traits:( NSUnboldFontMask | NSUnitalicFontMask ) weight:5 size:[pNSFont pointSize]];
			if ( pNewNSFont && pNewNSFont != pNSFont )
			{
				[pNewNSFont retain];
				pRet = pNewNSFont;
			}
		}
	}

	[pPool release];

	return pRet;
}

NSArray *NSFontManager_getAllFonts()
{
	NSArray *pRet = nil;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	NSFontManager *pFontManager = [NSFontManager sharedFontManager];
	if ( pFontManager )
	{
		NSArray *pFontNames = [pFontManager availableFonts];
		if ( pFontNames )
		{
			unsigned nCount = [pFontNames count];
			if ( nCount )
			{
				NSMutableArray *pFontArray = [NSMutableArray arrayWithCapacity:nCount];
				if ( pFontArray)
				{
					unsigned i = 0;
					for ( ; i < nCount; i++ )
					{
						NSFont *pCurrentFont = [NSFont fontWithName:static_cast< NSString* >( [pFontNames objectAtIndex:i] ) size:(float)12];
						if ( pCurrentFont )
						{
							// Fix bug 3097 by using the printer font when the
							// font is a bitmap font
							pCurrentFont = [pCurrentFont printerFont];
							if ( pCurrentFont )
								[pFontArray addObject:pCurrentFont];
						}
					}

					[pFontArray retain];
					pRet = pFontArray;
				}
			}
		}
	}

	[pPool release];

	return pRet;
}

sal_Bool NSFontManager_isFixedPitch( NSFont *pNSFont )
{
	sal_Bool bRet = sal_False;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager && [pFontManager traitsOfFont:static_cast< NSFont* >( pNSFont )] & NSFixedPitchFontMask )
			bRet = sal_True;
	}

	[pPool release];

	return bRet;
}

sal_Bool NSFontManager_isItalic( NSFont *pNSFont )
{
	sal_Bool bRet = sal_False;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager && [pFontManager traitsOfFont:static_cast< NSFont* >( pNSFont )] & NSItalicFontMask )
			bRet = sal_True;
	}

	[pPool release];

	return bRet;
}

FontWidth NSFontManager_widthOfFont( NSFont *pNSFont )
{
	FontWidth nRet = WIDTH_DONTKNOW;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager )
		{
			NSFontTraitMask nTraits = [pFontManager traitsOfFont:static_cast< NSFont* >( pNSFont )];
			if ( nTraits & NSCompressedFontMask )
				nRet = WIDTH_ULTRA_CONDENSED;
			else if ( nTraits & NSCondensedFontMask )
				nRet = WIDTH_CONDENSED;
			else if ( nTraits & NSNarrowFontMask )
				nRet = WIDTH_SEMI_CONDENSED;
			else if ( nTraits & NSExpandedFontMask )
				nRet = WIDTH_EXPANDED;
			else
				nRet = WIDTH_NORMAL;
		}
	}

	[pPool release];

	return nRet;
}

FontWeight NSFontManager_weightOfFont( NSFont *pNSFont )
{
	FontWeight nRet = WEIGHT_DONTKNOW;

	NSAutoreleasePool *pPool = [[NSAutoreleasePool alloc] init];

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager )
		{
			int nWeight = [pFontManager weightOfFont:static_cast< NSFont* >( pNSFont )];

			// Convert from NSFont weights to FontWeight values
			if ( nWeight <= 1 )
				nRet = WEIGHT_THIN;
			else if ( nWeight <= 6 )
				nRet = static_cast< FontWeight >( nWeight );
			else if ( nWeight <= 8 )
				nRet = WEIGHT_SEMIBOLD;
			else if ( nWeight <= 9 )
				nRet = WEIGHT_BOLD;
			else if ( nWeight <= 12 )
				nRet = WEIGHT_ULTRABOLD;
			else
				nRet = WEIGHT_BLACK;
		}
	}

	[pPool release];

	return nRet;
}
