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
#include <math.h>
#include "FilterInternal.hxx"
#include "TableStyle.hxx"
#include "DocumentElement.hxx"

#ifdef _MSC_VER
#include <minmax.h>
#endif

TableCellStyle::TableCellStyle(const WPXPropertyList &xPropList, const char *psName) :
	Style(psName),
        mPropList(xPropList)
{
}

void TableCellStyle::write(DocumentHandler &xHandler) const
{
	TagOpenElement styleOpen("style:style");
	styleOpen.addAttribute("style:name", getName());
	styleOpen.addAttribute("style:family", "table-cell");
	styleOpen.write(xHandler);

        // WLACH_REFACTORING: Only temporary.. a much better solution is to
        // generalize this sort of thing into the "Style" superclass
        WPXPropertyList stylePropList;
        WPXPropertyList::Iter i(mPropList);
        for (i.rewind(); i.next();)
        {
                if (strlen(i.key()) > 2 && strncmp(i.key(), "fo", 2) == 0)
                        stylePropList.insert(i.key(), i()->clone());
        }
        stylePropList.insert("fo:padding", "0.0382inch");
        xHandler.startElement("style:properties", stylePropList);
	xHandler.endElement("style:properties");

	xHandler.endElement("style:style");	
}

TableRowStyle::TableRowStyle(const WPXPropertyList &propList, const char *psName) :
	Style(psName),
        mPropList(propList)
{
}

void TableRowStyle::write(DocumentHandler &xHandler) const
{
	TagOpenElement styleOpen("style:style");
	styleOpen.addAttribute("style:name", getName());
	styleOpen.addAttribute("style:family", "table-row");
	styleOpen.write(xHandler);
	
        TagOpenElement stylePropertiesOpen("style:properties");
        if (mPropList["style:min-row-height"])
                stylePropertiesOpen.addAttribute("style:min-row-height", mPropList["style:min-row-height"]->getStr());
        else if (mPropList["style:row-height"])
                stylePropertiesOpen.addAttribute("style:row-height", mPropList["style:row-height"]->getStr());
        stylePropertiesOpen.write(xHandler);
        xHandler.endElement("style:properties");
	
	xHandler.endElement("style:style");		
}
	

TableStyle::TableStyle(const WPXPropertyList &xPropList, const WPXPropertyListVector &columns, const char *psName) : 
	Style(psName),
        mPropList(xPropList),
        mColumns(columns)
{
}

TableStyle::~TableStyle()
{
	typedef std::vector<TableCellStyle *>::iterator TCSVIter;
	for (TCSVIter iterTableCellStyles = mTableCellStyles.begin() ; iterTableCellStyles != mTableCellStyles.end(); iterTableCellStyles++)
		delete(*iterTableCellStyles);

}

void TableStyle::write(DocumentHandler &xHandler) const
{
	TagOpenElement styleOpen("style:style");
	styleOpen.addAttribute("style:name", getName());
	styleOpen.addAttribute("style:family", "table");
	if (getMasterPageName())
		styleOpen.addAttribute("style:master-page-name", getMasterPageName()->cstr());
	styleOpen.write(xHandler);

	TagOpenElement stylePropertiesOpen("style:properties");
        if (mPropList["table:align"])
                stylePropertiesOpen.addAttribute("table:align", mPropList["table:align"]->getStr());
	if (mPropList["fo:margin-left"])
		stylePropertiesOpen.addAttribute("fo:margin-left", mPropList["fo:margin-left"]->getStr());
	if (mPropList["fo:margin-right"])
		stylePropertiesOpen.addAttribute("fo:margin-right", mPropList["fo:margin-right"]->getStr());
	if (mPropList["style:width"])
		stylePropertiesOpen.addAttribute("style:width", mPropList["style:width"]->getStr());
	stylePropertiesOpen.write(xHandler);

	xHandler.endElement("style:properties");

	xHandler.endElement("style:style");
		
	int i=1;
        WPXPropertyListVector::Iter j(mColumns);
	for (j.rewind(); j.next();)
	{
		TagOpenElement styleOpen("style:style");
		WPXString sColumnName;
		sColumnName.sprintf("%s.Column%i", getName().cstr(), i);
		styleOpen.addAttribute("style:name", sColumnName);
		styleOpen.addAttribute("style:family", "table-column");
		styleOpen.write(xHandler);

                xHandler.startElement("style:properties", j());
		xHandler.endElement("style:properties");

		xHandler.endElement("style:style");

		i++;
	}

	typedef std::vector<TableRowStyle *>::const_iterator TRSVIter;
	for (TRSVIter iterTableRow = mTableRowStyles.begin() ; iterTableRow != mTableRowStyles.end(); iterTableRow++)
		(*iterTableRow)->write(xHandler);

	typedef std::vector<TableCellStyle *>::const_iterator TCSVIter;
	for (TCSVIter iterTableCell = mTableCellStyles.begin() ; iterTableCell != mTableCellStyles.end(); iterTableCell++)
		(*iterTableCell)->write(xHandler);
}
