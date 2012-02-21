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

#ifndef __SALOGL_COCOA_H__
#define __SALOGL_COCOA_H__

#include <sal/types.h>

#ifdef __cplusplus
BEGIN_C
#endif
#ifndef USE_CORETEXT_TEXT_RENDERING
SAL_DLLPRIVATE ATSFontRef NSFont_getATSFontRef( NSFont *pNSFont );
SAL_DLLPRIVATE CFStringRef NSFont_familyName( NSFont *pNSFont );
#endif	// !USE_CORETEXT_TEXT_RENDERING
SAL_DLLPRIVATE NSFont *NSFont_findFontWithStyle( NSFont *pNSFont, BOOL bBold, BOOL bItalic );
SAL_DLLPRIVATE NSArray *NSFontManager_getAllFonts();
SAL_DLLPRIVATE BOOL NSFontManager_isFixedPitch( NSFont *pNSFont );
SAL_DLLPRIVATE BOOL NSFontManager_isItalic( NSFont *pNSFont );
SAL_DLLPRIVATE int NSFontManager_widthOfFont( NSFont *pNSFont );
SAL_DLLPRIVATE int NSFontManager_weightOfFont( NSFont *pNSFont );
#ifdef __cplusplus
END_C
#endif

#endif
