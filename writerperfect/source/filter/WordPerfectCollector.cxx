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


#include <libwpd/libwpd.h>
#include <string.h> // for strcmp

#include "WordPerfectCollector.hxx"
#include "DocumentElement.hxx"
#include "TextRunStyle.hxx"
#include "FontStyle.hxx"
#include "ListStyle.hxx"
#include "PageSpan.hxx"
#include "SectionStyle.hxx"
#include "TableStyle.hxx"
#include "FilterInternal.hxx"
#include "WriterProperties.hxx"
#include "FontMap.hxx"

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

using namespace ::rtl;
using rtl::OUString;

using com::sun::star::xml::sax::XAttributeList;

_WriterDocumentState::_WriterDocumentState() :
	mbFirstElement(true),
	mbInFakeSection(false),
	mbListElementOpenedAtCurrentLevel(false),
	mbTableCellOpened(false),
	mbHeaderRow(false)
{
}

WordPerfectCollector::WordPerfectCollector() :
	mbUsed(false),
	miNumStyles(1),
	miNumSections(0),
	miCurrentNumColumns(1),
	mfSectionSpaceAfter(0.0f),
	miNumTables(0),
	miNumListStyles(0),
	mpCurrentContentElements(&mBodyElements),
	mpCurrentPageSpan(NULL),
	miNumPageStyles(0),
	mpCurrentListStyle(NULL),
	miCurrentListLevel(0),
	miLastListLevel(0),
	miLastListNumber(0),
	mbListContinueNumbering(false),
	mbListElementParagraphOpened(false),
	mbListElementOpened(false)
{
}

bool WordPerfectCollector::filter(WPXInputStream &input, Reference < XDocumentHandler > &xHandler)
{
	// The contract for WordPerfectCollector is that it will only be used once after it is
	// instantiated
	if (mbUsed)
		return false;

	mbUsed = true;

	// parse & write
 	if (!_parseSourceDocument(input))
		return false;
	if (!_writeTargetDocument(xHandler))
		return false;

 	// clean up the mess we made
 	WRITER_DEBUG_MSG(("WriterWordPerfect: Cleaning up our mess..\n"));

	WRITER_DEBUG_MSG(("Destroying the body elements\n"));
	for (vector<DocumentElement *>::iterator iterBody = mBodyElements.begin(); iterBody != mBodyElements.end(); iterBody++) {
		delete((*iterBody));
		(*iterBody) = NULL;
	}

	WRITER_DEBUG_MSG(("Destroying the styles elements\n"));
	for (vector<DocumentElement *>::iterator iterStyles = mStylesElements.begin(); iterStyles != mStylesElements.end(); iterStyles++) {
 		if (!(*iterStyles))
			WRITER_DEBUG_MSG(("NULL\n"));
		else
			(*iterStyles)->print();
 		delete (*iterStyles);
		(*iterStyles) = NULL; // we may pass over the same element again (in the case of headers/footers spanning multiple pages)
		                // so make sure we don't do a double del
	}

	WRITER_DEBUG_MSG(("Destroying the rest of the styles elements\n"));
	for (map<UTF8String, Style *, ltstr>::iterator iterTextStyle = mTextStyleHash.begin(); iterTextStyle != mTextStyleHash.end(); iterTextStyle++) {
		delete(iterTextStyle->second);
	}
	for (map<UTF8String, FontStyle *, ltstr>::iterator iterFont = mFontHash.begin(); iterFont != mFontHash.end(); iterFont++) {
		delete(iterFont->second);
	}

	for (vector<ListStyle *>::iterator iterListStyles = mListStyles.begin(); iterListStyles != mListStyles.end(); iterListStyles++) {
		delete((*iterListStyles));
	}
	for (vector<SectionStyle *>::iterator iterSectionStyles = mSectionStyles.begin(); iterSectionStyles != mSectionStyles.end(); iterSectionStyles++) {
		delete((*iterSectionStyles));
	}
	for (vector<TableStyle *>::iterator iterTableStyles = mTableStyles.begin(); iterTableStyles != mTableStyles.end(); iterTableStyles++) {
		delete((*iterTableStyles));
	}

	for (vector<PageSpan *>::iterator iterPageSpans = mPageSpans.begin(); iterPageSpans != mPageSpans.end(); iterPageSpans++) {
		delete((*iterPageSpans));
	}

 	return true;
 }

bool WordPerfectCollector::_parseSourceDocument(WPXInputStream &input)
{
	// create a header for the preamble + add some default settings to it

 	WRITER_DEBUG_MSG(("WriterWordPerfect: Attempting to process state\n"));
	bool bRetVal = true;
	try {
		WPDocument::parse(&input, static_cast<WPXHLListenerImpl *>(this));
	}
	catch (FileException)
		{
			WRITER_DEBUG_MSG(("Caught a file exception..\n"));
			bRetVal = false;
		}

	return bRetVal;
}

void WordPerfectCollector::_writeDefaultStyles(Reference < XDocumentHandler > &xHandler)
{
	TagOpenElement stylesOpenElement("office:styles");
	stylesOpenElement.write(xHandler);

	TagOpenElement defaultParagraphStyleOpenElement("style:default-style");
	defaultParagraphStyleOpenElement.addAttribute("style:family", "paragraph");
	defaultParagraphStyleOpenElement.write(xHandler);

	TagOpenElement defaultParagraphStylePropertiesOpenElement("style:properties");
	defaultParagraphStylePropertiesOpenElement.addAttribute("style:family", "paragraph");
	defaultParagraphStylePropertiesOpenElement.addAttribute("style:tab-stop-distance", "0.5inch");
	defaultParagraphStylePropertiesOpenElement.write(xHandler);
	TagCloseElement defaultParagraphStylePropertiesCloseElement("style:properties");
	defaultParagraphStylePropertiesCloseElement.write(xHandler);

	TagCloseElement defaultParagraphStyleCloseElement("style:default-style");
	defaultParagraphStyleCloseElement.write(xHandler);
	
	TagOpenElement standardStyleOpenElement("style:style");
        standardStyleOpenElement.addAttribute("style:name", "Standard");
        standardStyleOpenElement.addAttribute("style:family", "paragraph");
        standardStyleOpenElement.addAttribute("style:class", "text");
        standardStyleOpenElement.write(xHandler);
        TagCloseElement standardStyleCloseElement("style:style");
        standardStyleCloseElement.write(xHandler);

        TagOpenElement textBodyStyleOpenElement("style:style");
        textBodyStyleOpenElement.addAttribute("style:name", "Text Body");
        textBodyStyleOpenElement.addAttribute("style:family", "paragraph");
        textBodyStyleOpenElement.addAttribute("style:parent-style-name", "Standard");
        textBodyStyleOpenElement.addAttribute("style:class", "text");
        textBodyStyleOpenElement.write(xHandler);
        TagCloseElement textBodyStyleCloseElement("style:style");
        textBodyStyleCloseElement.write(xHandler);

        TagOpenElement tableContentsStyleOpenElement("style:style");
        tableContentsStyleOpenElement.addAttribute("style:name", "Table Contents");
        tableContentsStyleOpenElement.addAttribute("style:family", "paragraph");
        tableContentsStyleOpenElement.addAttribute("style:parent-style-name", "Text Body");
        tableContentsStyleOpenElement.addAttribute("style:class", "extra");
        tableContentsStyleOpenElement.write(xHandler);
        TagCloseElement tableContentsStyleCloseElement("style:style");
        tableContentsStyleCloseElement.write(xHandler);

        TagOpenElement tableHeadingStyleOpenElement("style:style");
        tableHeadingStyleOpenElement.addAttribute("style:name", "Table Heading");
        tableHeadingStyleOpenElement.addAttribute("style:family", "paragraph");
        tableHeadingStyleOpenElement.addAttribute("style:parent-style-name", "Table Contents");
        tableHeadingStyleOpenElement.addAttribute("style:class", "extra");
        tableHeadingStyleOpenElement.write(xHandler);
        TagCloseElement tableHeadingStyleCloseElement("style:style");
        tableHeadingStyleCloseElement.write(xHandler);

	TagCloseElement stylesCloseElement("office:styles");
	stylesCloseElement.write(xHandler);

}

void WordPerfectCollector::_writeContentPreamble(Reference < XDocumentHandler > &xHandler)
{
// the old content pre-amble: remove me after a few releases
	TagOpenElement documentContentOpenElement("office:document-content");
	documentContentOpenElement.addAttribute("xmlns:office", "http://openoffice.org/2000/office");
	documentContentOpenElement.addAttribute("xmlns:style", "http://openoffice.org/2000/style");
	documentContentOpenElement.addAttribute("xmlns:text", "http://openoffice.org/2000/text");
	documentContentOpenElement.addAttribute("xmlns:table", "http://openoffice.org/2000/table");
	documentContentOpenElement.addAttribute("xmlns:draw", "http://openoffice.org/2000/draw");
	documentContentOpenElement.addAttribute("xmlns:fo", "http://www.w3.org/1999/XSL/Format");
	documentContentOpenElement.addAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
	documentContentOpenElement.addAttribute("xmlns:number", "http://openoffice.org/2000/datastyle");
	documentContentOpenElement.addAttribute("xmlns:svg", "http://www.w3.org/2000/svg");
	documentContentOpenElement.addAttribute("xmlns:chart", "http://openoffice.org/2000/chart");
	documentContentOpenElement.addAttribute("xmlns:dr3d", "http://openoffice.org/2000/dr3d");
	documentContentOpenElement.addAttribute("xmlns:math", "http://www.w3.org/1998/Math/MathML");
	documentContentOpenElement.addAttribute("xmlns:form", "http://openoffice.org/2000/form");
	documentContentOpenElement.addAttribute("xmlns:script", "http://openoffice.org/2000/script");
	documentContentOpenElement.addAttribute("office:class", "text");
	documentContentOpenElement.addAttribute("office:version", "1.0");
	documentContentOpenElement.write(xHandler);
}

void WordPerfectCollector::_writeMasterPages(Reference < XDocumentHandler > &xHandler)
{
	SvXMLAttributeList * pAttrList = new SvXMLAttributeList;
	Reference < XAttributeList > xBlankAttrList ( pAttrList );

	xHandler->startElement(OUString::createFromAscii("office:master-styles"), xBlankAttrList);
	int pageNumber = 1;
	for (int i=0; i<mPageSpans.size(); i++)
	{
		bool bLastPage;
		(i == (mPageSpans.size() - 1)) ? bLastPage = true : bLastPage = false;
		mPageSpans[i]->writeMasterPages(pageNumber, i, bLastPage, xHandler);
		pageNumber += mPageSpans[i]->getSpan();
	}
	xHandler->endElement(OUString::createFromAscii("office:master-styles"));
}

void WordPerfectCollector::_writePageMasters(Reference < XDocumentHandler > &xHandler)
{
	int pageNumber = 1;
	for (int i=0; i<mPageSpans.size(); i++)
	{
		mPageSpans[i]->writePageMaster(i, xHandler);
	}
}

bool WordPerfectCollector::_writeTargetDocument(Reference < XDocumentHandler > &xHandler)
{
	WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: Printing out the header stuff..\n"));
 	SvXMLAttributeList * pAttrList = new SvXMLAttributeList;
	Reference < XAttributeList > xBlankAttrList ( pAttrList );

	WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: Start Document\n"));
	xHandler->startDocument();

	WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: preamble\n"));
	_writeContentPreamble(xHandler);

 	WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: Writing out the styles..\n"));

	// write out the font styles
	xHandler->startElement(OUString::createFromAscii("office:font-decls"), xBlankAttrList);
	for (map<UTF8String, FontStyle *, ltstr>::iterator iterFont = mFontHash.begin(); iterFont != mFontHash.end(); iterFont++) {
		iterFont->second->write(xHandler);
	}
	TagOpenElement symbolFontOpen("style:font-decl");
	symbolFontOpen.addAttribute("style:name", "StarSymbol");
	symbolFontOpen.addAttribute("fo:font-family", "StarSymbol");
	symbolFontOpen.addAttribute("style:font-charset", "x-symbol");
	symbolFontOpen.write(xHandler);
	TagCloseElement symbolFontClose("style:font-decl");
	symbolFontClose.write(xHandler);

	xHandler->endElement(OUString::createFromAscii("office:font-decls"));

	// write default styles
	_writeDefaultStyles(xHandler);

	// write automatic styles: which encompasses quite a bit
	xHandler->startElement(OUString::createFromAscii("office:automatic-styles"), xBlankAttrList);
	for (map<UTF8String, Style *, ltstr>::iterator iterTextStyle = mTextStyleHash.begin(); iterTextStyle != mTextStyleHash.end(); iterTextStyle++) {
		// writing out the paragraph styles
		if (strcmp((iterTextStyle->second)->getName(), "Standard") ||
		    !(dynamic_cast<ParagraphStyle *>(iterTextStyle->second))) {
			// don't write standard paragraph "no styles" style
			(iterTextStyle->second)->write(xHandler);
		}
	}

 	// writing out the sections styles
	for (vector<SectionStyle *>::iterator iterSectionStyles = mSectionStyles.begin(); iterSectionStyles != mSectionStyles.end(); iterSectionStyles++) {
		(*iterSectionStyles)->write(xHandler);
	}

	// writing out the lists styles
	for (vector<ListStyle *>::iterator iterListStyles = mListStyles.begin(); iterListStyles != mListStyles.end(); iterListStyles++) {
		(*iterListStyles)->write(xHandler);
	}

 	// writing out the table styles
	for (vector<TableStyle *>::iterator iterTableStyles = mTableStyles.begin(); iterTableStyles != mTableStyles.end(); iterTableStyles++) {
		(*iterTableStyles)->write(xHandler);
	}

	// writing out the page masters
	_writePageMasters(xHandler);

	xHandler->endElement(OUString::createFromAscii("office:automatic-styles"));

	_writeMasterPages(xHandler);

 	WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: Writing out the document..\n"));
 	// writing out the document
	xHandler->startElement(OUString::createFromAscii("office:body"), xBlankAttrList);

	for (vector<DocumentElement *>::iterator iterBodyElements = mBodyElements.begin(); iterBodyElements != mBodyElements.end(); iterBodyElements++) {
		(*iterBodyElements)->write(xHandler);
	}
 	WRITER_DEBUG_MSG(("WriterWordPerfect: Document Body: Finished writing all doc els..\n"));

	xHandler->endElement(OUString::createFromAscii("office:body"));
	xHandler->endElement(OUString::createFromAscii("office:document-content"));

	xHandler->endDocument();

	return true;
}

// _requestParagraphRunStyle: returns a paragraph style, if it already exists. creates it, adds it
// to the list of defined styles, and returns it otherwise.
ParagraphStyle * WordPerfectCollector::_requestParagraphStyle(const uint8_t iParagraphJustification,
							const float fMarginLeft, const float fMarginRight, const float fTextIndent,
							const float fLineSpacing, const float fSpacingBeforeParagraph,
							const float fSpacingAfterParagraph, const vector<WPXTabStop> &tabStops, 
							const bool bColumnBreak, const bool bPageBreak,
							const char *pParentName, const char *pName)
{
	if (mWriterDocumentState.mbFirstElement && mpCurrentContentElements == &mBodyElements)
	{
		WRITER_DEBUG_MSG(("WriterWordPerfect: If.. (mbFirstElement=%i)", mWriterDocumentState.mbFirstElement));
		ParagraphStyle * pParagraphStyle = new ParagraphStyle(iParagraphJustification,
								      fMarginLeft, fMarginRight, fTextIndent, fLineSpacing, fSpacingBeforeParagraph,
								      fSpacingAfterParagraph, tabStops, bColumnBreak, bPageBreak, "FS", pParentName);

		UTF8String sParagraphHashKey("P|FS");
		UTF8String sMasterPageName("Page Style 1");
		pParagraphStyle->setMasterPageName(sMasterPageName);
		mTextStyleHash[sParagraphHashKey] = pParagraphStyle;
		mWriterDocumentState.mbFirstElement = false;

		return pParagraphStyle;
 	}

	// else.. do the following:
	WRITER_DEBUG_MSG(("WriterWordPerfect: Else.. (mbFirstElement=%i)", mWriterDocumentState.mbFirstElement));
	UTF8String sTabList;
	sTabList.sprintf("%i", tabStops.size());
	for (int i=0; i<tabStops.size(); i++)
	{
		sTabList.sprintf("%s|%.4f", sTabList.getUTF8(), tabStops[i].m_position);
		switch (tabStops[i].m_alignment)
		{
		case LEFT:
			sTabList.sprintf(sTabList.getUTF8(), "L");
			break;
		case RIGHT:
			sTabList.sprintf(sTabList.getUTF8(), "R");
			break;
		case CENTER:
			sTabList.sprintf(sTabList.getUTF8(), "C");
			break;
		case DECIMAL:
			sTabList.sprintf(sTabList.getUTF8(), "D");
			break;
		case BAR: // BAR tab is not implemented in OOo.
		default:
			break;
		}
		if (tabStops[i].m_leaderCharacter != 0x0000)
			sTabList.sprintf("%s|0x%4x", sTabList.getUTF8(), tabStops[i].m_leaderCharacter);
	};
		
	UTF8String sParagraphHashKey;
	sParagraphHashKey.sprintf("P|%s|%i|%f|%f|%f|%f|%f|%f|%s|%i|%i",
					pParentName, iParagraphJustification, fMarginLeft, fMarginRight, fTextIndent, fLineSpacing,
					fSpacingBeforeParagraph, fSpacingAfterParagraph,sTabList.getUTF8(), bColumnBreak, bPageBreak);
	WRITER_DEBUG_MSG(("WriterWordPerfect: P|%s|%i|%f|%f|%f|%f|%f|%f|%s|%i|%i\n",
					pParentName, iParagraphJustification, fMarginLeft, fMarginRight, fTextIndent, fLineSpacing,
					fSpacingBeforeParagraph, fSpacingAfterParagraph, sTabList.getUTF8(), bColumnBreak, bPageBreak));
	WRITER_DEBUG_MSG(("WriterWordPerfect: Paragraph Hash Key: %s\n", sParagraphHashKey.getUTF8()));

	// Get the style
	ParagraphStyle * pParagraphStyle = NULL;
	if (mTextStyleHash.find(sParagraphHashKey) == mTextStyleHash.end()) {
		// allocate a new paragraph style
		const char *pFinalStyleName = NULL;
		UTF8String sName;
		sName.sprintf("S%i", miNumStyles); // have to define here so it doesn't go out of scope
		if (pName != NULL)
			pFinalStyleName = pName;
		else {
			pFinalStyleName = sName.getUTF8();
		}
		WRITER_DEBUG_MSG(("WriterWordPerfect: final paragraph style name: %s\n", pFinalStyleName));

		pParagraphStyle = new ParagraphStyle(iParagraphJustification,
						     fMarginLeft, fMarginRight, fTextIndent, fLineSpacing, fSpacingBeforeParagraph,
						     fSpacingAfterParagraph, tabStops, bColumnBreak, bPageBreak, pFinalStyleName, pParentName);

		miNumStyles++;
		mTextStyleHash[sParagraphHashKey] = pParagraphStyle;
		WRITER_DEBUG_MSG(("WriterWordPerfect: Successfully added to hash, returning it\n"));
	}
	else
	{
		pParagraphStyle = static_cast<ParagraphStyle *>(mTextStyleHash.find(sParagraphHashKey)->second);
	}

	return pParagraphStyle;
}


// _requestParagraphRunStyle: returns a paragraph style, if it already exists. creates it, adds it
// to the list of defined styles, and returns it otherwise.
ParagraphStyle * WordPerfectCollector::_requestListParagraphStyle(const ListStyle * pListStyle, const uint8_t iParagraphJustification,
					const float fMarginLeft, const float fMarginRight, const float fTextIndent, const float fLineSpacing,
					const float fSpacingBeforeParagraph, const float fSpacingAfterParagraph, const vector<WPXTabStop> &tabStops)
{
	if (mWriterDocumentState.mbFirstElement && mpCurrentContentElements == &mBodyElements)
	{
		WRITER_DEBUG_MSG(("WriterWordPerfect: If.. (mbFirstElement=%i)", mWriterDocumentState.mbFirstElement));

		ParagraphStyle * pListParagraphStyle = new ParagraphStyle(iParagraphJustification,
									  fMarginLeft, fMarginRight, fTextIndent, fLineSpacing,
									  fSpacingBeforeParagraph, fSpacingAfterParagraph, tabStops,
									  false, false, "FS1", "Standard");

		UTF8String sParagraphHashKey("P|ListFS");
		UTF8String sMasterPageName("Page Style 1");
		pListParagraphStyle->setMasterPageName(sMasterPageName);
		mTextStyleHash[sParagraphHashKey] = pListParagraphStyle;
		mWriterDocumentState.mbFirstElement = false;

		return pListParagraphStyle;
 	}

	UTF8String sTabList;
	sTabList.sprintf("%i", tabStops.size());
	for (int i=0; i<tabStops.size(); i++)
	{
		sTabList.sprintf("%s|%.4f", sTabList.getUTF8(), tabStops[i].m_position);
		switch (tabStops[i].m_alignment)
		{
		case LEFT:
			sTabList.sprintf(sTabList.getUTF8(), "L");
			break;
		case RIGHT:
			sTabList.sprintf(sTabList.getUTF8(), "R");
			break;
		case CENTER:
			sTabList.sprintf(sTabList.getUTF8(), "C");
			break;
		case DECIMAL:
			sTabList.sprintf(sTabList.getUTF8(), "D");
			break;
		case BAR: // BAR tab is not implemented in OOo.
		default:
			break;
		}
		if (tabStops[i].m_leaderCharacter != 0x0000)
			sTabList.sprintf("%s|0x%4x", sTabList.getUTF8(), tabStops[i].m_leaderCharacter);
	};
		
	UTF8String sParagraphHashKey;
	sParagraphHashKey.sprintf("P|%s|%i|%f|%f|%f|%f|%f|%f|%s", (const char *)pListStyle->getName(),
				     iParagraphJustification, fMarginLeft, fMarginRight, fTextIndent,
				     fLineSpacing, fSpacingBeforeParagraph, fSpacingAfterParagraph, sTabList.getUTF8());
	WRITER_DEBUG_MSG(("WriterWordPerfect: Paragraph Hash Key: %s\n", sParagraphHashKey.getUTF8()));

	// Get the style
	ParagraphStyle * pListParagraphStyle = NULL;
	if (mTextStyleHash.find(sParagraphHashKey) == mTextStyleHash.end()) {
		// allocate a new paragraph style
		UTF8String sName;
		sName.sprintf("S%i", miNumStyles);
		UTF8String sListStyleName(pListStyle->getName());

		pListParagraphStyle = new ParagraphStyle(iParagraphJustification,
							 fMarginLeft, fMarginRight, fTextIndent, fLineSpacing,
							 fSpacingBeforeParagraph, fSpacingAfterParagraph, tabStops, false, false,
							 sName.getUTF8(), "Standard");

		pListParagraphStyle->setListStyleName(sListStyleName);
		miNumStyles++;
		mTextStyleHash[sParagraphHashKey] = pListParagraphStyle;
	}

	WRITER_DEBUG_MSG(("WriterWordPerfect: Successfully added to hash, returning it\n"));

	return static_cast<ParagraphStyle *>(mTextStyleHash.find(sParagraphHashKey)->second);
}

// _allocateFontName: add a (potentially mapped) font style to the hash if it's not already there, do nothing otherwise
void WordPerfectCollector::_allocateFontName(const UTF8String & sFontName)
{
	if (mFontHash.find(sFontName) == mFontHash.end())
	{
		FontStyle *pFontStyle = new FontStyle(sFontName.getUTF8(), sFontName.getUTF8());
		mFontHash[sFontName] = pFontStyle;
	}
}

void WordPerfectCollector::openPageSpan(const int span, const bool isLastPageSpan,
					const float formLength, const float formWidth,
					const WPXFormOrientation formOrientation,
					const float marginLeft, const float marginRight,
					const float marginTop, const float marginBottom)
{
	PageSpan *pPageSpan = new PageSpan(span, formLength, formWidth, formOrientation, marginLeft, marginRight, marginTop, marginBottom);
	mPageSpans.push_back(pPageSpan);
	mpCurrentPageSpan = pPageSpan;
}

void WordPerfectCollector::openHeaderFooter(const WPXHeaderFooterType headerFooterType, const WPXHeaderFooterOccurence headerFooterOccurence)
{
	vector<DocumentElement *> * pHeaderFooterContentElements = new vector<DocumentElement *>;

	if (headerFooterType == HEADER)
	{
		switch (headerFooterOccurence)
		{
		case ALL:
		case ODD:
			WRITER_DEBUG_MSG(("WriterWordPerfect: Opening h_all or h_odd\n"));
			mpCurrentPageSpan->setHeaderContent(pHeaderFooterContentElements);
			break;
		case EVEN:
			WRITER_DEBUG_MSG(("WriterWordPerfect: Opening h_even\n"));
			mpCurrentPageSpan->setHeaderLeftContent(pHeaderFooterContentElements);
			break;
		}
	}
	else
	{
		switch (headerFooterOccurence)
		{
		case ALL:
		case ODD:
			WRITER_DEBUG_MSG(("WriterWordPerfect: Opening f_all or f_odd\n"));
			mpCurrentPageSpan->setFooterContent(pHeaderFooterContentElements);
			break;
		case EVEN:
			WRITER_DEBUG_MSG(("WriterWordPerfect: Opening f_even\n"));
			mpCurrentPageSpan->setFooterLeftContent(pHeaderFooterContentElements);
			break;
		}
	}

	mpCurrentContentElements = pHeaderFooterContentElements;
}

void WordPerfectCollector::closeHeaderFooter(const WPXHeaderFooterType headerFooterType,
					     const WPXHeaderFooterOccurence headerFooterOccurence)
{
	mpCurrentContentElements = &mBodyElements;
}

void WordPerfectCollector::openSection(const unsigned int numColumns, const vector<WPXColumnDefinition> &columns, const float spaceAfter)
{
/*	// libwpd pushes a section unconditionally to the listener (us) when it starts
	// sending the document message. this is primarily for AbiWord's convenience: its document
	// model requires that we open a section before writing actual content (e.g.: paragraphs, tables).
	// Since we don't actually want to start a new section unless the columns have changed in OOo,
	// we just "fake" a section opening in that case

	// Determine first whether columns have changed
	bool columnsChanged = false;
	if (miCurrentNumColumns != numColumns)
		columnsChanged = true;
	else if (mColumns.size() != columns.size())
		columnsChanged = true;
	else
	{
		for (int i=0; i<columns.size(); i++)
		{
			if (mColumns[i].m_leftGutter != columns[i].m_leftGutter)
				columnsChanged = true;
			else if (mColumns[i].m_width != columns[i].m_width)
				columnsChanged = true;
			else if (mColumns[i].m_rightGutter != columns[i].m_rightGutter)
				columnsChanged = true;
		}
	}

	if (columnsChanged)  {*/
		miCurrentNumColumns = numColumns;
		mColumns = columns;

		if (miCurrentNumColumns > 1)
		{
			miNumSections++;
			mfSectionSpaceAfter = spaceAfter;
			UTF8String sSectionName;
			sSectionName.sprintf("Section%i", miNumSections);
			WRITER_DEBUG_MSG(("WriterWordPerfect:  New Section: %s\n", sSectionName.getUTF8()));

			SectionStyle *pSectionStyle = new SectionStyle(miCurrentNumColumns, mColumns, sSectionName.getUTF8());
			mSectionStyles.push_back(pSectionStyle);

			TagOpenElement *pSectionOpenElement = new TagOpenElement("text:section");
			pSectionOpenElement->addAttribute("text:style-name", pSectionStyle->getName());
			pSectionOpenElement->addAttribute("text:name", pSectionStyle->getName());
			mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pSectionOpenElement));
		}
		else
			mWriterDocumentState.mbInFakeSection = true;
/*	}
	else {
		mWriterDocumentState.mbInFakeSection = true;
	}*/

}

void WordPerfectCollector::closeSection()
{
	if (!mWriterDocumentState.mbInFakeSection)
		mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:section")));
	else
		mWriterDocumentState.mbInFakeSection = false;

	// open as many paragraphs as needed to simulate section space after
	for (float f=0.0f; f<mfSectionSpaceAfter; f+=1.0f) {
		vector<WPXTabStop> dummyTabStops;
		openParagraph(WPX_PARAGRAPH_JUSTIFICATION_LEFT, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, dummyTabStops, false, false);
		closeParagraph();
	}
	mfSectionSpaceAfter = 0.0f;
}

void WordPerfectCollector::openParagraph(const uint8_t paragraphJustification,
					 const float marginLeftOffset, const float marginRightOffset, const float textIndent,
					 const float lineSpacing, const float spacingBeforeParagraph, const float spacingAfterParagraph,
					 const vector<WPXTabStop> &tabStops, const bool isColumnBreak, const bool isPageBreak)
{
	Style *pTextRunStyle;

	// FIXMENOW: What happens if we open a footnote inside a table? do we then inherit the footnote's style
	// from "Table Contents"
	if (mWriterDocumentState.mbTableCellOpened)
		if (mWriterDocumentState.mbHeaderRow)
			pTextRunStyle = _requestParagraphStyle(paragraphJustification, 0.0f, 0.0f, 0.0f, lineSpacing,
						spacingBeforeParagraph, spacingAfterParagraph, tabStops, false, false, "Table Heading");
		else
			pTextRunStyle = _requestParagraphStyle(paragraphJustification, 0.0f, 0.0f, 0.0f, lineSpacing,
						spacingBeforeParagraph, spacingAfterParagraph, tabStops, false, false, "Table Contents");
	else if (miCurrentNumColumns > 1)
		pTextRunStyle = _requestParagraphStyle(paragraphJustification, 0.0f, 0.0f, 0.0f, lineSpacing,
						spacingBeforeParagraph, spacingAfterParagraph, tabStops, isColumnBreak, isPageBreak, "Standard");
	else
		pTextRunStyle = _requestParagraphStyle(paragraphJustification, marginLeftOffset, marginRightOffset, textIndent, lineSpacing,
						spacingBeforeParagraph, spacingAfterParagraph, tabStops, isColumnBreak, isPageBreak, "Standard");


	// create a document element corresponding to the paragraph, and append it to our list of document elements
	TagOpenElement *pParagraphOpenElement = new TagOpenElement("text:p");
	pParagraphOpenElement->addAttribute("text:style-name", pTextRunStyle->getName());
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pParagraphOpenElement));
}

void WordPerfectCollector::closeParagraph()
{
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:p")));
}

void WordPerfectCollector::openSpan(const uint32_t textAttributeBits, const char *fontName, const float fontSize,
									const RGBSColor *fontColor, const RGBSColor *highlightColor)
{
	UTF8String sMappedFontName(mapFont(fontName));
	_allocateFontName(sMappedFontName);
	UTF8String sSpanHashKey;
	sSpanHashKey.sprintf("S|%i|%s|%f|%.2x%.2x%.2x%.2x|%.2x%.2x%.2x%.2x", textAttributeBits, sMappedFontName.getUTF8(),
						fontSize, fontColor->m_r, fontColor->m_g, fontColor->m_b, fontColor->m_s,
						(highlightColor?highlightColor->m_r:0xff), (highlightColor?highlightColor->m_g:0xff),
						(highlightColor?highlightColor->m_b:0xff), (highlightColor?highlightColor->m_s:0xff));
	WRITER_DEBUG_MSG(("WriterWordPerfect: Span Hash Key: %s\n", sSpanHashKey.getUTF8()));

	// Get the style
	Style * pTextRunStyle = NULL;
	if (mTextStyleHash.find(sSpanHashKey) == mTextStyleHash.end()) {
		// allocate a new paragraph style
		UTF8String sName;
		sName.sprintf("S%i", miNumStyles);
		pTextRunStyle = new SpanStyle(textAttributeBits, sMappedFontName.getUTF8(), fontSize, fontColor,
									  highlightColor, sName.getUTF8());

		miNumStyles++;
		mTextStyleHash[sSpanHashKey] = pTextRunStyle;
	}
	else {
		pTextRunStyle = mTextStyleHash.find(sSpanHashKey)->second; // yes, this could be optimized (see dup call above)
	}

	// create a document element corresponding to the paragraph, and append it to our list of document elements
	TagOpenElement *pSpanOpenElement = new TagOpenElement("text:span");
	pSpanOpenElement->addAttribute("text:style-name", pTextRunStyle->getName());
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pSpanOpenElement));
}

void WordPerfectCollector::closeSpan()
{
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:span")));
}

void WordPerfectCollector::defineOrderedListLevel(const int listID, const int listLevel, const WPXNumberingType listType,
						  const UCSString &textBeforeNumber, const UCSString &textAfterNumber,
						  int startingNumber)
{
	WRITER_DEBUG_MSG(("Define ordered list level (listid: %i)\n", listID));
 	OrderedListStyle *pOrderedListStyle = NULL;
	if (mpCurrentListStyle && mpCurrentListStyle->getListID() == listID)
		pOrderedListStyle = static_cast<OrderedListStyle *>(mpCurrentListStyle); // FIXME: using a dynamic cast here causes oo to crash?!
	// this rather appalling conditional makes sure we only start a new list (rather than continue an old
	// one) iff: (1) we have no prior list OR (2) the prior list is actually definitively different
	// from the list that is just being defined (listIDs differ) OR (3) we can tell that the user actually
	// is starting a new list at level 1 (and only level 1)
	if (pOrderedListStyle == NULL || pOrderedListStyle->getListID() != listID ||
	    (listLevel==1 && (startingNumber != (miLastListNumber+1))))
	{
		WRITER_DEBUG_MSG(("Attempting to create a new ordered list style (listid: %i)\n"));
		UTF8String sName;
		sName.sprintf("OL%i", miNumListStyles);
		miNumListStyles++;
		pOrderedListStyle = new OrderedListStyle(sName.getUTF8(), listID);
		mListStyles.push_back(static_cast<ListStyle *>(pOrderedListStyle));
		mpCurrentListStyle = static_cast<ListStyle *>(pOrderedListStyle);
		mbListContinueNumbering = false;
		miLastListNumber = 0;
	}
	else
		mbListContinueNumbering = true;

	pOrderedListStyle->updateListLevel(miCurrentListLevel, listType, textBeforeNumber, textAfterNumber, startingNumber);
}

void WordPerfectCollector::defineUnorderedListLevel(const int listID, const int listLevel, const UCSString &bullet)
{
	WRITER_DEBUG_MSG(("Define unordered list level (listid: %i)\n", listID));

 	UnorderedListStyle *pUnorderedListStyle = NULL;
	if (mpCurrentListStyle && mpCurrentListStyle->getListID() == listID)
		pUnorderedListStyle = static_cast<UnorderedListStyle *>(mpCurrentListStyle); // FIXME: using a dynamic cast here causes oo to crash?!

	if (pUnorderedListStyle == NULL) {
		WRITER_DEBUG_MSG(("Attempting to create a new unordered list style (listid: %i)\n", listID));
		UTF8String sName;
		sName.sprintf("UL%i", miNumListStyles);
		pUnorderedListStyle = new UnorderedListStyle(sName, listID);
		mListStyles.push_back(static_cast<ListStyle *>(pUnorderedListStyle));
		mpCurrentListStyle = static_cast<ListStyle *>(pUnorderedListStyle);
	}
	pUnorderedListStyle->updateListLevel(miCurrentListLevel, bullet);
}

void WordPerfectCollector::openOrderedListLevel(const int listID)
{
	WRITER_DEBUG_MSG(("Open ordered list level (listid: %i)\n", listID));
	miCurrentListLevel++;
	TagOpenElement *pListLevelOpenElement = new TagOpenElement("text:ordered-list");
	_openListLevel(pListLevelOpenElement);

	if (mbListContinueNumbering) {
		pListLevelOpenElement->addAttribute("text:continue-numbering", "true");
	}

	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pListLevelOpenElement));
}

void WordPerfectCollector::openUnorderedListLevel(const int listID)
{
	WRITER_DEBUG_MSG(("Open unordered list level (listid: %i)\n", listID));
	miCurrentListLevel++;
	TagOpenElement *pListLevelOpenElement = new TagOpenElement("text:unordered-list");
	_openListLevel(pListLevelOpenElement);

	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pListLevelOpenElement));
}

void WordPerfectCollector::_openListLevel(TagOpenElement *pListLevelOpenElement)
{
  	if (!mbListElementOpened && miCurrentListLevel > 1)
  	{
  		mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("text:list-item")));
  	}
	else if (mbListElementParagraphOpened)
	{
		mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:p")));
		mbListElementParagraphOpened = false;
	}

	if (miCurrentListLevel==1) {
		pListLevelOpenElement->addAttribute("text:style-name", mpCurrentListStyle->getName());
	}

	mbListElementOpened = false;
}

void WordPerfectCollector::closeOrderedListLevel()
{
	WRITER_DEBUG_MSG(("Close ordered list level)\n"));
	_closeListLevel("ordered-list");
}

void WordPerfectCollector::closeUnorderedListLevel()
{
	WRITER_DEBUG_MSG(("Close unordered list level\n"));
	_closeListLevel("unordered-list");
}

void WordPerfectCollector::_closeListLevel(const char *szListType)
{
	if (mbListElementOpened)
		mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:list-item")));

	miCurrentListLevel--;

	UTF8String sCloseElement;
	sCloseElement.sprintf("text:%s", szListType);
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement(sCloseElement.getUTF8())));

	if (miCurrentListLevel > 0)
		mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:list-item")));
	mbListElementOpened = false;
}

void WordPerfectCollector::openListElement(const uint8_t paragraphJustification,
					const float marginLeftOffset, const float marginRightOffset, const float textIndent,
					const float lineSpacing, const float spacingBeforeParagraph, const float spacingAfterParagraph, 
					const vector<WPXTabStop> &tabStops)
{
	miLastListLevel = miCurrentListLevel;
	if (miCurrentListLevel == 1)
		miLastListNumber++;

	if (mbListElementOpened)
		mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:list-item")));

 	ParagraphStyle * pListParagraphStyle = _requestListParagraphStyle(mpCurrentListStyle, paragraphJustification,
							marginLeftOffset, marginRightOffset, textIndent, lineSpacing,
							spacingBeforeParagraph, spacingAfterParagraph, tabStops);

	if (!pListParagraphStyle) {
		throw ParseException();
	}
	TagOpenElement *pOpenListElement = new TagOpenElement("text:list-item");
	TagOpenElement *pOpenListElementParagraph = new TagOpenElement("text:p");
	pOpenListElementParagraph->addAttribute("text:style-name", pListParagraphStyle->getName());
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pOpenListElement));
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pOpenListElementParagraph));

	mbListElementOpened = true;
	mbListElementParagraphOpened = true;
	mbListContinueNumbering = false;
}

void WordPerfectCollector::closeListElement()
{
	WRITER_DEBUG_MSG(("close list element\n"));

	// this code is kind of tricky, because we don't actually close the list element (because this list element
	// could contain another list level in OOo's implementation of lists). that is done in the closeListLevel
	// code (or when we open another list element)

	if (mbListElementParagraphOpened)
	{
		mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:p")));
		mbListElementParagraphOpened = false;
	}
}

void WordPerfectCollector::openFootnote(int number)
{
	WRITER_DEBUG_MSG(("open footnote\n"));
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("text:footnote")));

	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("text:footnote-citation")));
	UTF8String sFootnoteNumber;
	sFootnoteNumber.sprintf("%i", number);
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new CharDataElement(sFootnoteNumber.getUTF8())));
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:footnote-citation")));

	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("text:footnote-body")));

}

void WordPerfectCollector::closeFootnote()
{
	WRITER_DEBUG_MSG(("close footnote\n"));
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:footnote-body")));
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:footnote")));
}

void WordPerfectCollector::openEndnote(int number)
{
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("text:endnote")));

	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("text:endnote-citation")));
	UTF8String sEndnoteNumber;
	sEndnoteNumber.sprintf("%i", number);
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new CharDataElement(sEndnoteNumber.getUTF8())));
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:endnote-citation")));

	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("text:endnote-body")));

}
void WordPerfectCollector::closeEndnote()
{
	WRITER_DEBUG_MSG(("close endnote\n"));
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:endnote-body")));
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:endnote")));
}

void WordPerfectCollector::openTable(const uint8_t tablePositionBits, const float marginLeftOffset, const float marginRightOffset,
				     const float leftOffset, const vector < WPXColumnDefinition > &columns)
{
	miNumTables++;

	UTF8String sTableName;
	sTableName.sprintf("Table%i", miNumTables);
	WRITER_DEBUG_MSG(("WriterWordPerfect:  New Table: %s\n", sTableName.getUTF8()));

	// FIXME: we base the table style off of the page's margin left, ignoring (potential) wordperfect margin
	// state which is transmitted inside the page. could this lead to unacceptable behaviour?
	TableStyle *pTableStyle = new TableStyle(mpCurrentPageSpan->getMarginLeft(), mpCurrentPageSpan->getMarginRight(), marginLeftOffset, marginRightOffset, tablePositionBits, leftOffset, columns, sTableName.getUTF8());

	if (mWriterDocumentState.mbFirstElement && mpCurrentContentElements == &mBodyElements)
	{
		UTF8String sMasterPageName("Page Style 1");
		pTableStyle->setMasterPageName(sMasterPageName);
		mWriterDocumentState.mbFirstElement = false;
	}

	mTableStyles.push_back(pTableStyle);

	mpCurrentTableStyle = pTableStyle;

	TagOpenElement *pTableOpenElement = new TagOpenElement("table:table");

	pTableOpenElement->addAttribute("table:name", sTableName.getUTF8());
	pTableOpenElement->addAttribute("table:style-name", sTableName.getUTF8());
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pTableOpenElement));

	for (int i=0; i<pTableStyle->getNumColumns(); i++) {
		TagOpenElement *pTableColumnOpenElement = new TagOpenElement("table:table-column");
		UTF8String sColumnStyleName;
		sColumnStyleName.sprintf("%s.Column%i", sTableName.getUTF8(), (i+1));
		pTableColumnOpenElement->addAttribute("table:style-name", sColumnStyleName.getUTF8());
		mpCurrentContentElements->push_back(pTableColumnOpenElement);

		TagCloseElement *pTableColumnCloseElement = new TagCloseElement("table:table-column");
		mpCurrentContentElements->push_back(pTableColumnCloseElement);
	}
}

void WordPerfectCollector::openTableRow(const float height, const bool isMinimumHeight, const bool isHeaderRow)
{
	if (isHeaderRow)
	{
		mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("table:table-header-rows")));
		mWriterDocumentState.mbHeaderRow = true;
	}

	UTF8String sTableRowStyleName;
	sTableRowStyleName.sprintf("%s.Row%i", (const char *)mpCurrentTableStyle->getName(), mpCurrentTableStyle->getNumTableRowStyles());
	TableRowStyle *pTableRowStyle = new TableRowStyle(height, isMinimumHeight, isHeaderRow, sTableRowStyleName);
	mpCurrentTableStyle->addTableRowStyle(pTableRowStyle);
	
	TagOpenElement *pTableRowOpenElement = new TagOpenElement("table:table-row");
	pTableRowOpenElement->addAttribute("table:style-name", sTableRowStyleName);
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pTableRowOpenElement));
}

void WordPerfectCollector::closeTableRow()
{
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("table:table-row")));
	if (mWriterDocumentState.mbHeaderRow)
	{
		mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("table:table-header-rows")));
		mWriterDocumentState.mbHeaderRow = false;
	}
}

void WordPerfectCollector::openTableCell(const uint32_t col, const uint32_t row, const uint32_t colSpan, const uint32_t rowSpan,
					 const uint8_t borderBits, const RGBSColor * cellFgColor, const RGBSColor * cellBgColor,
					 const RGBSColor * cellBorderColor,
					 const WPXVerticalAlignment cellVerticalAlignment)
{
	float fLeftBorderThickness = (borderBits & WPX_TABLE_CELL_LEFT_BORDER_OFF) ? 0.0f : 0.0007f;
	float fRightBorderThickness = (borderBits & WPX_TABLE_CELL_RIGHT_BORDER_OFF) ? 0.0f : 0.0007f;
	float fTopBorderThickness = (borderBits & WPX_TABLE_CELL_TOP_BORDER_OFF) ? 0.0f : 0.0007f;
	float fBottomBorderThickness = (borderBits & WPX_TABLE_CELL_BOTTOM_BORDER_OFF) ? 0.0f : 0.0007f;
	WRITER_DEBUG_MSG(("WriterWordPerfect: Borderbits=%d\n", borderBits));

	UTF8String sTableCellStyleName; 
	sTableCellStyleName.sprintf( "%s.Cell%i", (const char *)mpCurrentTableStyle->getName(), mpCurrentTableStyle->getNumTableCellStyles());
	TableCellStyle *pTableCellStyle = new TableCellStyle(fLeftBorderThickness, fRightBorderThickness,
							     fTopBorderThickness, fBottomBorderThickness,
							     cellFgColor, cellBgColor, cellBorderColor, cellVerticalAlignment,
							     sTableCellStyleName);
	mpCurrentTableStyle->addTableCellStyle(pTableCellStyle);

	TagOpenElement *pTableCellOpenElement = new TagOpenElement("table:table-cell");
	rtl::OString sNumColsSpanned = utf8_itoa(colSpan);
	rtl::OString sNumRowsSpanned = utf8_itoa(rowSpan);
	pTableCellOpenElement->addAttribute("table:style-name", sTableCellStyleName);
	pTableCellOpenElement->addAttribute("table:number-columns-spanned", sNumColsSpanned);
	pTableCellOpenElement->addAttribute("table:number-rows-spanned", sNumRowsSpanned);
	pTableCellOpenElement->addAttribute("table:value-type", "string");
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(pTableCellOpenElement));

	mWriterDocumentState.mbTableCellOpened = true;
}

void WordPerfectCollector::closeTableCell()
{
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("table:table-cell")));
	mWriterDocumentState.mbTableCellOpened = false;
}

void WordPerfectCollector::insertCoveredTableCell(const uint32_t col, const uint32_t row)
{
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("table:covered-table-cell")));
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("table:covered-table-cell")));
}

void WordPerfectCollector::closeTable()
{
	WRITER_DEBUG_MSG(("WriterWordPerfect: Closing Table\n"));
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("table:table")));
}


void WordPerfectCollector::insertTab()
{
	WRITER_DEBUG_MSG(("WriterWordPerfect: Insert Tab\n"));
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("text:tab-stop")));
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:tab-stop")));
}

void WordPerfectCollector::insertLineBreak()
{
	WRITER_DEBUG_MSG(("WriterWordPerfect: Insert Line Break\n"));
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagOpenElement("text:line-break")));
	mpCurrentContentElements->push_back(static_cast<DocumentElement *>(new TagCloseElement("text:line-break")));
}

void WordPerfectCollector::insertText(const UCSString &text)
{
	WRITER_DEBUG_MSG(("WriterWordPerfect: Insert Text\n"));

	DocumentElement *pText = new TextElement(text);
	mpCurrentContentElements->push_back(pText);
}
