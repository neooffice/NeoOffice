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

/* "This product is not manufactured, approved, or supported by 
 * Corel Corporation or Corel Corporation Limited."
 */
#include "FilterInternal.hxx"
#include "SectionStyle.hxx"
#include "DocumentElement.hxx"
#include <math.h>

#ifdef _MSC_VER
double rint(double x);
#endif /* _WIN32 */

const float fDefaultSideMargin = 1.0f; // inches
const float fDefaultPageWidth = 8.5f; // inches (OOo required default: we will handle this later)
const float fDefaultPageHeight = 11.0f; // inches

SectionStyle::SectionStyle(const WPXPropertyList &xPropList, 
                           const WPXPropertyListVector &xColumns, 
                           const char *psName) : 
        Style(psName),
        mPropList(xPropList),
        mColumns(xColumns)
{
}

void SectionStyle::write(DocumentHandler &xHandler) const
{
	TagOpenElement styleOpen("style:style");
	styleOpen.addAttribute("style:name", getName());
	styleOpen.addAttribute("style:family", "section");
	styleOpen.write(xHandler);

	// if the number of columns is <= 1, we will never come here. This is only an additional check
	if (mColumns.count() > 1)
	{		
		// style properties
                xHandler.startElement("style:properties", mPropList);

		// column properties
                WPXPropertyList columnProps;
                columnProps.insert("fo:column-count", (int)mColumns.count());
                xHandler.startElement("style:columns", columnProps);
	
                WPXPropertyListVector::Iter i(mColumns);
                for (i.rewind(); i.next();)
		{
                        xHandler.startElement("style:column", i());
                        xHandler.endElement("style:column");
		}

                xHandler.endElement("style:columns");
                xHandler.endElement("style:properties");
	}

	xHandler.endElement("style:style");
}
