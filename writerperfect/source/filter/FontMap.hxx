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
 *  GNU General Public License Version 2.1
 *  =============================================
 *  Copyright 2002-2003 William Lachance (william.lachance@sympatico.ca)
 *  http://libwpd.sourceforge.net
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
 *  =================================================
 *  Modified November 2004 by Patrick Luby. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/


#ifndef _FONTMAP_H
#define _FONTMAP_H

#include <stdio.h>
#include <libwpd/libwpd_support.h>
const char * mapFont(const char *font);

#ifdef FONTMAPPING
#define FONT_SERIF     "Times New Roman"
#define FONT_SANSSERIF "Helvetica"
#define FONT_MONOSPACE "Courier"

typedef struct _FontMapping FontMapping;
struct _FontMapping 
{
	char *pWordPerfectFont;
	char *pConvertedFont; 
};

const FontMapping fontConversionTable[] = 
{ 
	{ "Allegro", FONT_SERIF },
	{ "Amelia", "Goth" },
	{ "Bank Gothic", FONT_SANSSERIF },
	{ "Bernhard", FONT_MONOSPACE },
	{ "Bernhard Fashion", "Goth" },
	{ "Bernhard Modern", FONT_SERIF },
	{ "Bitstream Arrus", FONT_SERIF },
	{ "Bitstream Cooper", FONT_MONOSPACE },
	{ "Bitstream Oz Handicraft", FONT_SANSSERIF },
	{ "Bodoni", FONT_SERIF },
	{ "Bremen", FONT_SERIF },
	{ "Broadway", FONT_SANSSERIF },
	{ "BroadwayEngraved", FONT_SANSSERIF },
	{ "Brush", "Chancery L" },
	{ "BrushScript", "Chancery L" },
	{ "Caslon Openface", "ChevaraOutline" },
	{ "Century", "Century Schoolbook" },
	{ "Charlesworth", FONT_SERIF },
	{ "ChelmsfordBook", FONT_SERIF },
	{ "Classical Garamond", FONT_SERIF },
	{ "CommercialScript", "Zapf Chancery" },
	{ "CooperBlack", FONT_MONOSPACE },
	{ "Copperplate Gothic", FONT_MONOSPACE },
	{ "Courier", FONT_MONOSPACE },
	{ "Dauphin Normal", FONT_SERIF },
	{ "Engravers' Gothic", FONT_SANSSERIF },
	{ "Engravers' Old English", FONT_SERIF },
	{ "Enviro D", FONT_SANSSERIF },
	{ "Eurostile", FONT_SANSSERIF },
	{ "Flareserif", FONT_SERIF },
	{ "Futura", FONT_MONOSPACE },
	{ "Futura MdCn BT", FONT_MONOSPACE },
	{ "Geometric Slabserif", FONT_MONOSPACE },
	{ "GoldMine", FONT_MONOSPACE },
	{ "Goudy Handtooled", FONT_SERIF },
	{ "Goudy Old Style", FONT_SERIF },
	{ "Helvetica", FONT_SANSSERIF },
	{ "Helve", FONT_SANSSERIF },
	{ "Hobo", FONT_SANSSERIF },
	{ "Humanist", FONT_SANSSERIF },
	{ "Informal", FONT_SANSSERIF },
	{ "ITC American Typewriter", FONT_MONOSPACE },
	{ "ITC Benguiat", FONT_SERIF },
	{ "ITC Benguiat Book", FONT_SERIF },
	{ "ITC Cheltenham", FONT_SERIF },
	{ "ITC Cheltenham Book", FONT_SERIF },
	{ "ITC Galliard", FONT_SERIF },
	{ "ITC Goudy Sans", FONT_SANSSERIF },
	{ "ITC Kabel Book", FONT_SANSSERIF },
	{ "ITC Korinna", FONT_SANSSERIF },
	{ "ITC Korinna Kursiv", "Zapf Chancery" },
	{ "ITC Souvenir", FONT_SERIF },
	{ "ITC Zapf Chancery", "Zapf Chancery" },
	{ "Kabel Bd", FONT_SANSSERIF },
	{ "Kids", FONT_SANSSERIF },
	{ "Lapidary", FONT_SERIF },
	{ "Latin Wide D", FONT_SERIF },
	{ "Letter Gothic 12 Pitch", FONT_SERIF },
	{ "Lithograph", FONT_SANSSERIF },
	{ "Mister Earl", FONT_SANSSERIF },
	{ "Mona Lisa Recut", "ChevaraOutline" },
	{ "MurrayHill", "Zapf Chancery" },
	{ "Nevison Casual D", "Zapf Chancery" },
	{ "OCR-A", FONT_MONOSPACE },
	{ "OldEnglish", FONT_SERIF },
	{ "OldTown", FONT_MONOSPACE },
	{ "Onyx", FONT_SERIF },
	{ "Parisian", FONT_SERIF },
	{ "Pipeline", FONT_SERIF },
	{ "Poster Bodoni", FONT_SERIF },
	{ "Ribbon", "Zapf Chancery" },
	{ "Roman", FONT_SERIF },
	{ "Serifa", FONT_MONOSPACE },
	{ "Snell", "Zapf Chancery" },
	{ "Staccato", "Zapf Chancery" },
	{ "Stencil", FONT_MONOSPACE },
	{ "Stop D", FONT_SANSSERIF },
	{ "Swiss", FONT_SANSSERIF },
	{ "Swiss Black", FONT_SANSSERIF },
	{ "Symbol", "OpenSymbol" },
	{ "Technical", FONT_SANSSERIF },
	{ "Times New Roman", FONT_SERIF },
	{ "Transitional", FONT_SERIF },
	{ "Transitional Cursive", "Zapf Chancery" },
	{ "Typo Upright", "Zapf Chancery" },
	{ "Umbra", FONT_SANSSERIF },
	{ "University Roman", FONT_SERIF },
	{ "URW Wood Type D", FONT_MONOSPACE },
	{ "Venetian", FONT_SERIF },
	{ "WP BoxDrawing", "OpenSymbol" },
	{ "Zapf Elliptical", FONT_SERIF },
	{ "Zapf Humanist", FONT_SANSSERIF },
	{ "Zurich", FONT_SANSSERIF },
	{ "Zurich Black Extended", FONT_SANSSERIF },
	{ NULL, NULL } 
};
#endif
#endif
