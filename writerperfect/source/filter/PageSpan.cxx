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

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */
#include "FilterInternal.hxx"
#include "PageSpan.hxx"
#include "DocumentElement.hxx"

const float fDefaultPageWidth = 8.5f; // inches (OOo required default: we will handle this later)
const float fDefaultPageHeight = 11.0f; // inches

PageSpan::PageSpan(const WPXPropertyList &xPropList) :
#if 0
const int iSpan, const float fFormLength, const float fFormWidth, const WPXFormOrientation fFormOrientation,
                   const float fLeftMargin, const float fRightMargin, const float fTopMargin, const float fBottomMargin):
	miSpan(iSpan),
	mfFormLength(fFormLength),
	mfFormWidth(fFormWidth),
	mfFormOrientation(fFormOrientation),
	mfMarginLeft(fLeftMargin),
	mfMarginRight(fRightMargin),
	mfMarginTop(fTopMargin),
	mfMarginBottom(fBottomMargin),
#endif
        mxPropList(xPropList),
	mpHeaderContent(NULL),
	mpFooterContent(NULL),
	mpHeaderLeftContent(NULL),
	mpFooterLeftContent(NULL)
{
}

PageSpan::~PageSpan()
{
	delete mpHeaderContent;
	delete mpHeaderLeftContent;
	delete mpFooterContent;
	delete mpFooterLeftContent;
}

int PageSpan::getSpan() const
{ 
        if (mxPropList["libwpd:num-pages"]) 
                return mxPropList["libwpd:num-pages"]->getInt(); 

        return 0; // should never happen
}

float PageSpan::getMarginLeft() const
{ 
        if (mxPropList["fo:margin-left"])
                return mxPropList["fo:margin-left"]->getFloat();

         return 0.0f; 
}

float PageSpan::getMarginRight() const
{ 
        if (mxPropList["fo:margin-right"])
                return mxPropList["fo:margin-right"]->getFloat();

         return 0.0f; 
}

void PageSpan::writePageMaster(const int iNum, DocumentHandler &xHandler) const
{
        WPXPropertyList propList;
        
	WPXString sPageMasterName; 
        sPageMasterName.sprintf("PM%i", iNum);
        propList.insert("style:name", sPageMasterName);
	propList.insert("style:name", sPageMasterName);
	xHandler.startElement("style:page-master", propList);

        xHandler.startElement("style:properties", mxPropList);

        xHandler.endElement("style:properties");
        xHandler.endElement("style:page-master");
}

void PageSpan::writeMasterPages(const int iStartingNum, const int iPageMasterNum, const bool bLastPageSpan, 
                                DocumentHandler &xHandler) const
{
        WPXPropertyList propList; // scratch space

	int iSpan = 0;
	(bLastPageSpan) ? iSpan = 1 : iSpan = getSpan();

	for (int i=iStartingNum; i<(iStartingNum+iSpan); i++)
	{
		TagOpenElement masterPageOpen("style:master-page");
		WPXString sMasterPageName;
		sMasterPageName.sprintf("Page Style %i", i);
		WPXString sPageMasterName;
		sPageMasterName.sprintf("PM%i", iPageMasterNum);
                propList.insert("style:name", sMasterPageName);
		propList.insert("style:page-master-name", sPageMasterName);
		if (!bLastPageSpan)
		{
			WPXString sNextMasterPageName;
			sNextMasterPageName.sprintf("Page Style %i", (i+1));
                        propList.insert("style:next-style-name", sNextMasterPageName);
		}
                xHandler.startElement("style:master-page", propList);

		if (mpHeaderContent)
			_writeHeaderFooter("style:header", *mpHeaderContent, xHandler);
		if (mpHeaderLeftContent)
			_writeHeaderFooter("style:header-left", *mpHeaderLeftContent, xHandler);
		if (mpFooterContent)
			_writeHeaderFooter("style:footer", *mpFooterContent, xHandler);
		if (mpFooterLeftContent)
			_writeHeaderFooter("style:footer-left", *mpFooterLeftContent, xHandler);

                xHandler.endElement("style:master-page");
	}

}

void PageSpan::_writeHeaderFooter(const char *headerFooterTagName,
				  const std::vector<DocumentElement *> & headerFooterContent,
				  DocumentHandler &xHandler) const
{
	TagOpenElement headerFooterOpen(headerFooterTagName);
	headerFooterOpen.write(xHandler);
	for (std::vector<DocumentElement *>::const_iterator iter = headerFooterContent.begin();
	     iter != headerFooterContent.end();
	     iter++) {
		(*iter)->write(xHandler);
	}
	TagCloseElement headerFooterClose(headerFooterTagName);
	headerFooterClose.write(xHandler);
}

