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
 *  Copyright 2004 Fridrich Strba (fridrich.strba@bluewin.ch)
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


#include "FilterInternal.hxx"
#include "SectionStyle.hxx"
#include "DocumentElement.hxx"
#include <math.h>

using namespace ::rtl;
using rtl::OUString;

const float fDefaultSideMargin = 1.0f; // inches 
const float fDefaultPageWidth = 8.5f; // inches (OOo required default: we will handle this later)
const float fDefaultPageHeight = 11.0f; // inches

SectionStyle::SectionStyle(const int iNumColumns, const vector<WPXColumnDefinition> &columns, const char *psName) : Style(psName),
	miNumColumns(iNumColumns)
{

	for (int i=0; i<columns.size(); i++)
		mColumns.push_back(columns[i]);
	WRITER_DEBUG_MSG(("WriterWordPerfect: Created a new set of section props with this no. of columns: %i and this name: %s\n", 
	       (int)miNumColumns, (const char *)getName()));	
}

void SectionStyle::write(Reference < XDocumentHandler > &xHandler) const
{
	TagOpenElement styleOpen("style:style");
	styleOpen.addAttribute("style:name", getName());
	styleOpen.addAttribute("style:family", "section");
	styleOpen.write(xHandler);

	// if miNumColumns <= 1, we will never come here. This is only additional check
	if (miNumColumns > 1)
	{		
		// style properties
		TagOpenElement stylePropertiesOpen("style:properties");
		stylePropertiesOpen.addAttribute("text:dont-balance-text-columns", "false");
		stylePropertiesOpen.write(xHandler);

		// column properties
		TagOpenElement columnsOpen("style:columns");
		UTF8String sColumnCount;
		sColumnCount.sprintf("%i", miNumColumns);
		columnsOpen.addAttribute("fo:column-count", sColumnCount.getUTF8());
		columnsOpen.write(xHandler);
	
		UTF8String sRelWidth, sMarginLeft, sMarginRight;
		for (int i=0; i<miNumColumns; i++)
		{
			TagOpenElement columnOpen("style:column");
			// The "style:rel-width" is expressed in twips (1440 twips per inch) and includes the left and right Gutter
			sRelWidth.sprintf("%i*", (int)rint(mColumns[i].m_width * 1440.0f));
			columnOpen.addAttribute("style:rel-width", sRelWidth.getUTF8());
			sMarginLeft.sprintf("%.4finch", mColumns[i].m_leftGutter);
			columnOpen.addAttribute("fo:margin-left", sMarginLeft.getUTF8());
			sMarginRight.sprintf("%.4finch", mColumns[i].m_rightGutter);
			columnOpen.addAttribute("fo:margin-right", sMarginRight.getUTF8());
			columnOpen.write(xHandler);
			
			TagCloseElement columnClose("style:column");
			columnClose.write(xHandler);
		}
	}

	xHandler->endElement(OUString::createFromAscii("style:columns"));
	xHandler->endElement(OUString::createFromAscii("style:properties"));
	xHandler->endElement(OUString::createFromAscii("style:style"));
}
