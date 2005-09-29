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
 *  Copyright 2005 by Patrick Luby (patrick.luby@planamesa.com)
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

#import <Cocoa/Cocoa.h>
#import "salgdi3_cocoa.h"

id NSFont_create( CFStringRef aFontName, long nSize )
{
	if ( aFontName && nSize )
		return [NSFont fontWithName:(NSString *)aFontName size:(float)nSize];
	else
		return nil;
}

void NSFont_release( id pNSFont )
{
	if ( pNSFont )
		[(NSFont *)pNSFont release];
}

BOOL NSFontManager_isFixedPitch( id pNSFont )
{
	if ( pNSFont )
		return [pNSFont isFixedPitch];
	else
		return FALSE;
}

BOOL NSFontManager_isItalic( id pNSFont )
{
	BOOL bRet = FALSE;

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager )
			bRet = ( [pFontManager traitsOfFont:(NSFont *)pNSFont] & NSItalicFontMask );
	}

	return bRet;
}

int NSFontManager_widthOfFont( id pNSFont )
{
	int nRet = 0;

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager )
		{
			NSFontTraitMask nTraits = [pFontManager traitsOfFont:(NSFont *)pNSFont];
			if ( nTraits & NSCompressedFontMask )
				nRet = 1;
			else if ( nTraits & NSCondensedFontMask )
				nRet = 3;
			else if ( nTraits & NSNarrowFontMask )
				nRet = 4;
			else if ( nTraits & NSExpandedFontMask )
				nRet = 7;
			else
				nRet = 5;
		}
	}

	return nRet;
}
int NSFontManager_weightOfFont( id pNSFont )
{
	int nRet = 0;

	if ( pNSFont )
	{
		NSFontManager *pFontManager = [NSFontManager sharedFontManager];
		if ( pFontManager )
		{
			int nWeight = [pFontManager weightOfFont:(NSFont *)pNSFont];

			// Convert from NSFont weights to FontWeight values
			if ( nWeight <= 1 )
				nRet = 1;
			else if ( nWeight <= 6 )
				nRet = nWeight;
			else if ( nWeight <= 8 )
				nRet = 7;
			else if ( nWeight <= 9 )
				nRet = 8;
			else if ( nWeight <= 12 )
				nRet = 9;
			else
				nRet = 10;
		}
	}

	return nRet;
}
