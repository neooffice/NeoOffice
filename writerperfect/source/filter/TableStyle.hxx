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


#ifndef _TABLESTYLE_H
#define _TABLESTYLE_H
#include <libwpd/libwpd.h>
#include <vector>

#include "Style.hxx"
#include "WriterProperties.hxx"

using com::sun::star::uno::Reference;
using com::sun::star::xml::sax::XDocumentHandler;

class DocumentElement;

class TableCellStyle : public Style
{
public:
	TableCellStyle(const float fLeftBorderThickness, const float fRightBorderThickness, 
		       const float fTopBorderThickness, const float fBottomBorderThickness, 
		       const RGBSColor *pFgColor, const RGBSColor *pBgColor, const RGBSColor * pBorderColor,
		       const WPXVerticalAlignment cellVerticalAlignment, const char *psName);
	virtual void write(Reference < XDocumentHandler > &xHandler) const;
private:
	float mfLeftBorderThickness;
	float mfRightBorderThickness;
	float mfTopBorderThickness;
	float mfBottomBorderThickness;
	WPXVerticalAlignment mCellVerticalAlignment;
	RGBSColor m_fgColor;
	RGBSColor m_bgColor;
	RGBSColor m_borderColor;
};

class TableRowStyle : public Style
{
public:
	TableRowStyle(const float fHeight, const bool bIsMinimumHeight, const bool bIsHeaderRow, const char *psName);
	virtual void write(Reference < XDocumentHandler > &xHandler) const;
private:
	bool mbIsHeaderRow, mbIsMinimumHeight;
	float mfHeight;
};

class TableStyle : public Style, public TopLevelElementStyle
{
 public:
	TableStyle(const float fDocumentMarginLeft, const float fDocumentMarginRight, 
		   const float fMarginLeftOffset, const float fMarginRightOffset,
		   const uint8_t iTablePositionBits, const float fLeftOffset, 
		   const vector < WPXColumnDefinition > &columns, 
		   const char *psName);
	~TableStyle();
	virtual void write(Reference < XDocumentHandler > &xHandler) const;
	const int getNumColumns() const { return miNumColumns; }
	void addTableCellStyle(TableCellStyle *pTableCellStyle) { mTableCellStyles.push_back(pTableCellStyle); }
	int getNumTableCellStyles() { return mTableCellStyles.size(); }
	void addTableRowStyle(TableRowStyle *pTableRowStyle) { mTableRowStyles.push_back(pTableRowStyle); }
	int getNumTableRowStyles() { return mTableRowStyles.size(); }
private:	
	float mfDocumentMarginLeft, mfDocumentMarginRight;
	float mfMarginLeftOffset, mfMarginRightOffset;
	vector< WPXColumnDefinition > mColumns;
	unsigned int miTablePositionBits;
	float mfLeftOffset;
	vector<TableCellStyle *> mTableCellStyles;
	vector<TableRowStyle *> mTableRowStyles;
	int miNumColumns;
};

#endif
