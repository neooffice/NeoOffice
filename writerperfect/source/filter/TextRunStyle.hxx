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


#ifndef _TEXTRUNSTYLE_H
#define _TEXTRUNSTYLE_H
#include <libwpd/libwpd.h>

#include "Style.hxx"

using com::sun::star::uno::Reference;
using com::sun::star::xml::sax::XDocumentHandler;

class TagOpenElement;
class DocumentElement;

class SpanStyle : public Style
{
public:
	SpanStyle(const uint32_t iTextAttributeBits, const char *pFontName, const float fFontSize, const RGBSColor *pFontColor,
		  const RGBSColor *pHighlightColor, const char *psName);
	virtual void write(Reference < XDocumentHandler > &xHandler) const;
	const int getTextAttributeBits() const { return miTextAttributeBits; }
	const UTF8String & getFontName() const { return msFontName; }
	const float getFontSize() const { return mfFontSize; }

	void _addTextProperties(TagOpenElement *pStylePropertiesOpenElement) const;

private:
	int miTextAttributeBits;
	UTF8String msFontName;
	float mfFontSize;
	RGBSColor m_fontColor;
	RGBSColor m_highlightColor;
};

class ParagraphStyle : public Style, public TopLevelElementStyle
{
public:
	ParagraphStyle(const uint8_t iParagraphJustification,
			const float fMarginLeft, const float fMarginRight, const float fTextIndent, const float fLineSpacing,
			const float fSpacingBeforeParagraph, const float fSpacingAfterParagraph, const vector<WPXTabStop> &tabStops, 
			const bool bColumnBreak, const bool bPageBreak, const char *psName, const char *psParentName);

	virtual ~ParagraphStyle();

	void setListStyleName(UTF8String &sListStyleName) { delete mpsListStyleName ; mpsListStyleName = new UTF8String(sListStyleName); }
	virtual void write(Reference < XDocumentHandler > &xHandler) const;
	const virtual bool isParagraphStyle() const { return true; }

private:
	UTF8String msParentName;
	UTF8String *mpsListStyleName;
	float mfMarginLeft;
	float mfMarginRight;
	float mfTextIndent;
	float mfLineSpacing;
	float mfSpacingBeforeParagraph;
	float mfSpacingAfterParagraph;
	uint8_t miParagraphJustification;
	vector<WPXTabStop> mTabStops;
	int miNumTabStops;
	bool mbColumnBreak;
	bool mbPageBreak;
};
#endif
