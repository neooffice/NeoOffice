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
 *  Copyright 2002-2003 Net Integration Technologies (http://www.net-itech.com) 
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


#ifndef _WORDPERFECTCOLLECTOR_H
#define _WORDPERFECTCOLLECTOR_H
#include "SectionStyle.hxx"

#include <libwpd/libwpd.h>
#include <vector>
#include <map>
#include <stack>

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

using com::sun::star::uno::Reference;
using com::sun::star::xml::sax::XDocumentHandler;

using namespace std;

class DocumentElement;
class TagOpenElement;
class FontStyle;
class ListStyle;

class ParagraphStyle;
class TableStyle;
class PageSpan;

// the state we use for writing the final document
typedef struct _WriterDocumentState WriterDocumentState;
struct _WriterDocumentState
{
	_WriterDocumentState();
		
	bool mbFirstElement;
	bool mbInFakeSection;
	bool mbListElementOpenedAtCurrentLevel;
	bool mbTableCellOpened;
	bool mbHeaderRow;
};

enum WriterListType { unordered, ordered };

struct ltstr
{
  bool operator()(const UTF8String & s1, const UTF8String & s2) const
  {
    return strcmp(s1.getUTF8(), s2.getUTF8()) < 0;
  }
};

class WordPerfectCollector : public WPXHLListenerImpl
{
public:
	WordPerfectCollector();
	virtual ~WordPerfectCollector() {}
	bool filter(WPXInputStream &input, Reference < XDocumentHandler > &xHandler);

        virtual void setDocumentMetaData(const UCSString &author, const UCSString &subject,
                                         const UCSString &publisher, const UCSString &category,
                                         const UCSString &keywords, const UCSString &language,
                                         const UCSString &abstract, const UCSString &descriptiveName,
                                         const UCSString &descriptiveType) {}


	virtual void startDocument() {}
	virtual void endDocument() {}

	virtual void openPageSpan(const int span, const bool isLastPageSpan,
				  const float formLength, const float formWidth,
				  const WPXFormOrientation formOrientation,
				  const float marginLeft, const float marginRight,
				  const float marginTop, const float marginBottom);
	virtual void closePageSpan() {}
	virtual void openHeaderFooter(const WPXHeaderFooterType headerFooterType, const WPXHeaderFooterOccurence headerFooterOccurence);
	virtual void closeHeaderFooter(const WPXHeaderFooterType headerFooterType, const WPXHeaderFooterOccurence headerFooterOccurence);

	virtual void openSection(const unsigned int numColumns, const vector<WPXColumnDefinition> &columns, const float spaceAfter);
	virtual void closeSection();
	virtual void openParagraph(const uint8_t paragraphJustification, const float marginLeftOffset, const float marginRightOffset,
						const float textIndent, const float lineSpacing, const float spacingBeforeParagraph,
						const float spacingAfterParagraph, const vector<WPXTabStop> &tabStops,
						const bool isColumnBreak, const bool isPageBreak);
	virtual void closeParagraph();
	virtual void openSpan(const uint32_t textAttributeBits, const char *fontName,
				  const float fontSize, const RGBSColor *fontColor, const RGBSColor *highlightColor);
	virtual void closeSpan();

	virtual void insertTab();
	virtual void insertText(const UCSString &text);
	virtual void insertLineBreak();

	virtual void defineOrderedListLevel(const int listID, const int listLevel, const WPXNumberingType listType,
					    const UCSString &textBeforeNumber, const UCSString &textAfterNumber,
					    const int startingNumber);
	virtual void defineUnorderedListLevel(const int listID, const int listLevel, const UCSString &bullet);
	virtual void openOrderedListLevel(const int listID);
	virtual void openUnorderedListLevel(const int listID);
	virtual void closeOrderedListLevel();
	virtual void closeUnorderedListLevel();
	virtual void openListElement(const uint8_t paragraphJustification, const float marginLeftOffset, const float marginRightOffset,
						const float textIndent, const float lineSpacing, const float spacingBeforeParagraph,
						const float spacingAfterParagraph, const vector<WPXTabStop> &tabStops);
	virtual void closeListElement();

	virtual void openFootnote(int number);
	virtual void closeFootnote();
	virtual void openEndnote(int number);
	virtual void closeEndnote();

 	virtual void openTable(const uint8_t tablePositionBits, const float marginLeftOffset, const float marginRightOffset,
			       const float leftOffset, const vector < WPXColumnDefinition > &columns);
	virtual void openTableRow(const float height, const bool isMinimumHeight, const bool isHeaderRow);
	virtual void closeTableRow();
 	virtual void openTableCell(const uint32_t col, const uint32_t row, const uint32_t colSpan, const uint32_t rowSpan,
				   const uint8_t borderBits, const RGBSColor * cellFgColor, const RGBSColor * cellBgColor,
				   const RGBSColor * cellBorderColor,
				   const WPXVerticalAlignment cellVerticalAlignment);
	virtual void closeTableCell();
	virtual void insertCoveredTableCell(const uint32_t col, const uint32_t row);
	virtual void closeTable();

protected:
	void _resetDocumentState();
	bool _parseSourceDocument(WPXInputStream &input);
	bool _writeTargetDocument(Reference < XDocumentHandler > &xHandler);
	// _requestParagraphStyle: returns a text run style, if it already exists. creates it, adds it
	// to the list of defined styles, and returns it otherwise.
	ParagraphStyle * _requestParagraphStyle(const uint8_t iParagraphJustification,
							const float fMarginLeftOffset, const float fMarginRightOffset, const float fTextIndent,
							const float fLineSpacing, const float fSpacingBeforeParagraph, const float fSpacingAfterParagraph,
							const vector<WPXTabStop> &tabStops, const bool bColumnBreak,
							const bool bPageBreak, const char *pParentName = NULL, const char *pName = NULL);
	ParagraphStyle * _requestListParagraphStyle(const ListStyle * pListStyle, const uint8_t iParagraphJustification,
							const float fMarginLeftOffset, const float fMarginRightOffset, const float fTextIndent,
						    	const float fLineSpacing, const float fSpacingBeforeParagraph, const float fSpacingAfterParagraph,
							const vector<WPXTabStop> &tabStops);
	void _writeContentPreamble(Reference < XDocumentHandler > &xHandler);
	void _writeDefaultStyles(Reference < XDocumentHandler > &xHandler);
	void _writeMasterPages(Reference < XDocumentHandler > &xHandler);
	void _writePageMasters(Reference < XDocumentHandler > &xHandler);
	void _allocateFontName(const UTF8String &);

private:
	void _openListLevel(TagOpenElement *pListLevelOpenElement);
	void _closeListLevel(const char *szListType);

	bool mbUsed; // whether or not it has been before (you can only use me once!)

	WriterDocumentState mWriterDocumentState;

	// paragraph + span styles
	map<UTF8String, Style *, ltstr> mTextStyleHash;
	unsigned int miNumStyles;

	// font styles
	map<UTF8String, FontStyle *, ltstr> mFontHash;

	// section styles
	unsigned int miNumSections;
	vector<SectionStyle *> mSectionStyles;
	int miCurrentNumColumns;
	vector<WPXColumnDefinition> mColumns;
	float mfSectionSpaceAfter;

	// table styles
	unsigned int miNumTables;
	vector<TableStyle *> mTableStyles;

	// list styles
	unsigned int miNumListStyles;

	// style elements
	vector<DocumentElement *> mStylesElements;
	// content elements
	vector<DocumentElement *> mBodyElements;
	// the current set of elements that we're writing to
	vector<DocumentElement *> * mpCurrentContentElements;

	// page state
	vector<PageSpan *> mPageSpans;
	PageSpan *mpCurrentPageSpan;
	int miNumPageStyles;

	// list styles / state
	ListStyle *mpCurrentListStyle;
	unsigned int miCurrentListLevel;
	unsigned int miLastListLevel;
	unsigned int miLastListNumber;
	vector<ListStyle *> mListStyles;
	bool mbListContinueNumbering;
	bool mbListElementOpened;
	bool mbListElementParagraphOpened;

	// table state
	TableStyle *mpCurrentTableStyle;
};
#endif
