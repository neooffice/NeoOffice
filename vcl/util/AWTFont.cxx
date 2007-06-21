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
 *  Patrick Luby, June 2007
 *
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2007 by Patrick Luby (patrick.luby@planamesa.com)
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

#define _SV_AWTFONT_CXX

#include <map>

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include "AWTFont_cocoa.h"

static ::std::map< ATSFontRef, CGFontRef > aATSFontMap;
static ::osl::Mutex aATSFontMutex;

using namespace osl;

// ============================================================================

CGFontRef CreateCachedCGFont( ATSFontRef aATSFont )
{
	CGFontRef aFont = NULL;

	MutexGuard aGuard( aATSFontMutex );

	::std::map< ATSFontRef, CGFontRef >::iterator it = aATSFontMap.find( aATSFont );
	if ( it != aATSFontMap.end() )
	{
		aFont = it->second;
	}
	else
	{
		aFont = CGFontCreateWithPlatformFont( (void *)&aATSFont );
		if ( aFont )
			aATSFontMap[ aATSFont ] = aFont;
	}

	if ( aFont )
		CGFontRetain( aFont );

	return aFont;
}
