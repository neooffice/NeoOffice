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


#include <math.h>
#include "FilterInternal.hxx"
#include "TableStyle.hxx"
#include "DocumentElement.hxx"

#ifdef _MSC_VER
#include <minmax.h>
#endif

using namespace ::rtl;
using rtl::OUString;

TableCellStyle::TableCellStyle(const float fLeftBorderThickness, const float fRightBorderThickness, 
				const float fTopBorderThickness, const float fBottomBorderThickness, 
				const RGBSColor *pFgColor, const RGBSColor *pBgColor, const RGBSColor *pBorderColor,
				const WPXVerticalAlignment cellVerticalAlignment, const char *psName) :
	Style(psName),
	mfLeftBorderThickness(fLeftBorderThickness),
	mfRightBorderThickness(fRightBorderThickness),
	mfTopBorderThickness(fTopBorderThickness),
	mfBottomBorderThickness(fBottomBorderThickness),
	mCellVerticalAlignment(cellVerticalAlignment)
{
	if (pFgColor != NULL) {
		m_fgColor.m_r = pFgColor->m_r;
		m_fgColor.m_g = pFgColor->m_g;
		m_fgColor.m_b = pFgColor->m_b;
		m_fgColor.m_s = pFgColor->m_s;
	}
	else {
		m_fgColor.m_r = m_fgColor.m_g = m_fgColor.m_b = 0xFF;
		m_fgColor.m_s = 0x64; // 100%
	}
	if (pBgColor != NULL) {
		m_bgColor.m_r = pBgColor->m_r;
		m_bgColor.m_g = pBgColor->m_g;
		m_bgColor.m_b = pBgColor->m_b;
		m_bgColor.m_s = pBgColor->m_s;
	}
	else {
		m_bgColor.m_r = m_bgColor.m_g = m_bgColor.m_b = 0xFF;
		m_bgColor.m_s = 0x64; // 100%
	}
	m_borderColor.m_r = pBorderColor->m_r;
	m_borderColor.m_g = pBorderColor->m_g;
	m_borderColor.m_b = pBorderColor->m_b;
	m_borderColor.m_s = pBorderColor->m_s;
}

void TableCellStyle::write(Reference < XDocumentHandler > &xHandler) const
{
	TagOpenElement styleOpen("style:style");
	styleOpen.addAttribute("style:name", getName());
	styleOpen.addAttribute("style:family", "table-cell");
	styleOpen.write(xHandler);

	TagOpenElement stylePropertiesOpen("style:properties");
	switch (mCellVerticalAlignment)
	{
	case TOP:
		stylePropertiesOpen.addAttribute("fo:vertical-align", "top");
		break;
	case MIDDLE:
		stylePropertiesOpen.addAttribute("fo:vertical-align", "middle");
		break;
	case BOTTOM:
		stylePropertiesOpen.addAttribute("fo:vertical-align", "bottom");
		break;
	case FULL: // OOo does not have the full vertical alignment
		break;
	default:
		break;
	}
	
	UTF8String sBackgroundColor;
	float fgAmount = (float)m_fgColor.m_s/100.0f;
	float bgAmount = max(((float)m_bgColor.m_s-(float)m_fgColor.m_s)/100.0f, 0.0f);

	int bgRed = min((int)(((float)m_fgColor.m_r*fgAmount)+((float)m_bgColor.m_r*bgAmount)), 255);
	int bgGreen = min((int)(((float)m_fgColor.m_g*fgAmount)+((float)m_bgColor.m_g*bgAmount)), 255);
	int bgBlue = min((int)(((float)m_fgColor.m_b*fgAmount)+((float)m_bgColor.m_b*bgAmount)), 255);
	sBackgroundColor.sprintf("#%.2x%.2x%.2x", bgRed, bgGreen, bgBlue);
	stylePropertiesOpen.addAttribute("fo:background-color", sBackgroundColor.getUTF8());
	stylePropertiesOpen.addAttribute("fo:padding", "0.0382inch");

	UTF8String sBorderColor;
	float borderShading = (float)((float)m_borderColor.m_s/100.0f); //convert the percents to float between 0 and 1
	// Mix borderShading amount of given color with (1-borderShading) of White (#ffffff)
	int borderRed = (int)0xFF + (int)((float)m_borderColor.m_r*borderShading) - (int)((float)0xFF*borderShading);
	int borderGreen = (int)0xFF + (int)((float)m_borderColor.m_g*borderShading) - (int)((float)0xFF*borderShading);
	int borderBlue = (int)0xFF + (int)((float)m_borderColor.m_b*borderShading) - (int)((float)0xFF*borderShading);
	sBorderColor.sprintf("#%.2x%.2x%.2x", borderRed, borderGreen, borderBlue);
	
	UTF8String sBorderLeft;
	sBorderLeft.sprintf("%finch solid %s", mfLeftBorderThickness, sBorderColor.getUTF8());
	stylePropertiesOpen.addAttribute("fo:border-left", sBorderLeft.getUTF8());
	UTF8String sBorderRight;
	sBorderRight.sprintf("%finch solid %s", mfRightBorderThickness, sBorderColor.getUTF8());
	stylePropertiesOpen.addAttribute("fo:border-right", sBorderRight.getUTF8());
	UTF8String sBorderTop;
	sBorderTop.sprintf("%finch solid %s", mfTopBorderThickness, sBorderColor.getUTF8());
	stylePropertiesOpen.addAttribute("fo:border-top", sBorderTop.getUTF8());
	UTF8String sBorderBottom;
	sBorderBottom.sprintf("%finch solid %s", mfBottomBorderThickness, sBorderColor.getUTF8());
	stylePropertiesOpen.addAttribute("fo:border-bottom", sBorderBottom.getUTF8());
	stylePropertiesOpen.write(xHandler);
	xHandler->endElement(OUString::createFromAscii("style:properties"));

	xHandler->endElement(OUString::createFromAscii("style:style"));
	
//  	xHandler->endElement(OUString::createFromAscii("<style:style style:name=\"%s\" style:family=\"%s\"><style:properties fo:background-color=\"#%.2x%.2x%.2x\" fo:padding=\"0.0382inch\" fo:border-left=\"%finch solid #000000\" fo:border-right=\"%finch solid #000000\" fo:border-top=\"%finch solid #000000\" fo:border-bottom=\"%finch solid #000000\"/></style:style>\n", getName(), "table-cell", 
// 			  m_fgColor.m_r, m_fgColor.m_g, m_fgColor.m_b,
// 			  mfLeftBorderThickness, mfRightBorderThickness, 
// 			  mfTopBorderThickness, mfBottomBorderThickness);
}

TableRowStyle::TableRowStyle(const float fHeight, const bool bIsMinimumHeight, const bool bIsHeaderRow, const char *psName):
	Style(psName),
	mfHeight(fHeight),
	mbIsMinimumHeight(bIsMinimumHeight),
	mbIsHeaderRow(bIsHeaderRow)
{
}

void TableRowStyle::write(Reference < XDocumentHandler > &xHandler) const
{
	TagOpenElement styleOpen("style:style");
	styleOpen.addAttribute("style:name", getName());
	styleOpen.addAttribute("style:family", "table-row");
	styleOpen.write(xHandler);
	
	if ((mfHeight != 0.0f) || (!mbIsMinimumHeight))
	{
		TagOpenElement stylePropertiesOpen("style:properties");
		UTF8String sRowHeight;
		sRowHeight.sprintf("%finch", mfHeight);
		if (mbIsMinimumHeight)
			stylePropertiesOpen.addAttribute("style:min-row-height", sRowHeight.getUTF8());
		else
			stylePropertiesOpen.addAttribute("style:row-height", sRowHeight.getUTF8());
		stylePropertiesOpen.write(xHandler);
		xHandler->endElement(OUString::createFromAscii("style:properties"));
	}
	
	xHandler->endElement(OUString::createFromAscii("style:style"));
		
}
	

TableStyle::TableStyle(const float fDocumentMarginLeft, const float fDocumentMarginRight, 
		       const float fMarginLeftOffset, const float fMarginRightOffset,
		       const uint8_t iTablePositionBits, const float fLeftOffset,
		       const vector < WPXColumnDefinition > &columns, const char *psName) : 
	Style(psName),
	mfDocumentMarginLeft(fDocumentMarginLeft),
	mfDocumentMarginRight(fDocumentMarginRight),
	mfMarginLeftOffset(fMarginLeftOffset),
	mfMarginRightOffset(fMarginRightOffset),
	miTablePositionBits(iTablePositionBits),
	mfLeftOffset(fLeftOffset),
	miNumColumns(columns.size())

{
	WRITER_DEBUG_MSG(("WriterWordPerfect: Created a new set of table props with this no. of columns repeated: %i and this name: %s\n",
	       (int)miNumColumns, (const char *)getName()));

	typedef vector<WPXColumnDefinition>::const_iterator CDVIter;
	for (CDVIter iterColumns = columns.begin() ; iterColumns != columns.end(); iterColumns++)
	{
		mColumns.push_back((*iterColumns));
	}
}

TableStyle::~TableStyle()
{
	typedef vector<TableCellStyle *>::iterator TCSVIter;
	for (TCSVIter iterTableCellStyles = mTableCellStyles.begin() ; iterTableCellStyles != mTableCellStyles.end(); iterTableCellStyles++)
		delete(*iterTableCellStyles);

}

void TableStyle::write(Reference < XDocumentHandler > &xHandler) const
{
	TagOpenElement styleOpen("style:style");
	styleOpen.addAttribute("style:name", getName());
	styleOpen.addAttribute("style:family", "table");
	if (getMasterPageName())
		styleOpen.addAttribute("style:master-page-name", getMasterPageName()->getUTF8());
	styleOpen.write(xHandler);

	TagOpenElement stylePropertiesOpen("style:properties");

	UTF8String sTableMarginLeft;
	UTF8String sTableMarginRight;
	UTF8String sTableAlignment;
	char *pTableAlignment = NULL;
	if (miTablePositionBits == WPX_TABLE_POSITION_ALIGN_WITH_LEFT_MARGIN) {
		sTableAlignment.sprintf("left");
		sTableMarginLeft.sprintf("0inch");
	}
	else if (miTablePositionBits == WPX_TABLE_POSITION_ALIGN_WITH_RIGHT_MARGIN) {
		sTableAlignment.sprintf("right");
	}
	else if (miTablePositionBits == WPX_TABLE_POSITION_CENTER_BETWEEN_MARGINS) {
		sTableAlignment.sprintf("center");
	}
 	else if (miTablePositionBits == WPX_TABLE_POSITION_ABSOLUTE_FROM_LEFT_MARGIN) {
		sTableAlignment.sprintf("left");
		sTableMarginLeft.sprintf("%finch", (mfLeftOffset-mfDocumentMarginLeft+mfMarginLeftOffset));
 	}
	else if (miTablePositionBits == WPX_TABLE_POSITION_FULL) {
		sTableAlignment.sprintf("margins");
		sTableMarginLeft.sprintf("%finch", mfMarginLeftOffset);
		sTableMarginRight.sprintf("%finch", mfMarginRightOffset);
	}		
	stylePropertiesOpen.addAttribute("table:align", sTableAlignment.getUTF8());
	if (sTableMarginLeft.getUTF8())
		stylePropertiesOpen.addAttribute("fo:margin-left", sTableMarginLeft.getUTF8());
	if (sTableMarginRight.getUTF8())
		stylePropertiesOpen.addAttribute("fo:margin-right", sTableMarginRight.getUTF8());

 	float fTableWidth = 0;
 	typedef vector<WPXColumnDefinition>::const_iterator CDVIter;
 	for (CDVIter iterColumns2 = mColumns.begin() ; iterColumns2 != mColumns.end(); iterColumns2++)
 	{
 		fTableWidth += (*iterColumns2).m_width;
 	}
	UTF8String sTableWidth;
	sTableWidth.sprintf("%finch", fTableWidth);
	stylePropertiesOpen.addAttribute("style:width", sTableWidth.getUTF8());
	stylePropertiesOpen.write(xHandler);

	xHandler->endElement(OUString::createFromAscii("style:properties"));

	xHandler->endElement(OUString::createFromAscii("style:style"));

		 
// 	if (getMasterPageName()) {
// 		xHandler->endElement(OUString::createFromAscii("<style:style style:name=\"%s\" style:family=\"%s\" style:master-page-name=\"%s\"><style:properties table:align=\"%s\" %s %s style:width=\"%finch\"/></style:style>\n", getName(), "table", getMasterPageName()->getUTF8(), pTableAlignment, psTableMarginLeft, psTableMarginRight, fTableWidth);
// 	}
// 	else {
// 		xHandler->endElement(OUString::createFromAscii("<style:style style:name=\"%s\" style:family=\"%s\"><style:properties table:align=\"%s\" %s %s style:width=\"%finch\"/></style:style>\n", getName(), "table", pTableAlignment, psTableMarginLeft, psTableMarginRight, fTableWidth);
// 	}

	int i=1;
	typedef vector<WPXColumnDefinition>::const_iterator CDVIter;
	for (CDVIter iterColumns = mColumns.begin() ; iterColumns != mColumns.end(); iterColumns++)
	{
		TagOpenElement styleOpen("style:style");
		UTF8String sColumnName;
		sColumnName.sprintf("%s.Column%i", (const char *)getName(), i);
		styleOpen.addAttribute("style:name", sColumnName.getUTF8());
		styleOpen.addAttribute("style:family", "table-column");
		styleOpen.write(xHandler);

		TagOpenElement stylePropertiesOpen("style:properties");
		UTF8String sColumnWidth;
		sColumnWidth.sprintf("%finch", (*iterColumns).m_width);
		stylePropertiesOpen.addAttribute("style:column-width", sColumnWidth.getUTF8());
		stylePropertiesOpen.write(xHandler);
		xHandler->endElement(OUString::createFromAscii("style:properties"));

		xHandler->endElement(OUString::createFromAscii("style:style"));

//xHandler->endElement(OUString::createFromAscii("<style:style style:name=\"%s.Column%i\" style:family=\"%s\"><style:properties style:column-width=\"%finch\"/></style:style>\n", getName(), i, "table-column", (*iter).m_width);
		i++;
	}

	typedef vector<TableRowStyle *>::const_iterator TRSVIter;
	for (TRSVIter iterTableRow = mTableRowStyles.begin() ; iterTableRow != mTableRowStyles.end(); iterTableRow++)
		(*iterTableRow)->write(xHandler);

	typedef vector<TableCellStyle *>::const_iterator TCSVIter;
	for (TCSVIter iterTableCell = mTableCellStyles.begin() ; iterTableCell != mTableCellStyles.end(); iterTableCell++)
		(*iterTableCell)->write(xHandler);
}
