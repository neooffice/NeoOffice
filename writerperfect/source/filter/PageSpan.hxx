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
#ifndef _PAGESPAN_H
#define _PAGESPAN_H
#include <libwpd/libwpd.h>
#include <vector>

class DocumentElement;
class DocumentHandler;

class PageSpan
{
public:
	PageSpan(const WPXPropertyList &xPropList);
	virtual ~PageSpan();
	void writePageMaster(const int iNum, DocumentHandler &xHandler) const;
	void writeMasterPages(const int iStartingNum, const int iPageMasterNum, const bool bLastPageSpan, DocumentHandler &xHandler) const;
	int getSpan() const;
#if 0
	float getFormLength() { return mfFormLength; }
	float getFormWidth() { return mfFormWidth; }
	WPXFormOrientation getFormOrientation() { return mfFormOrientation; }
#endif
	float getMarginLeft() const;
	float getMarginRight() const;

	const std::vector<DocumentElement *> * getHeaderContent() const { return mpHeaderContent; }
	void setHeaderContent(std::vector<DocumentElement *> * pHeaderContent) { mpHeaderContent = pHeaderContent; }
	void setFooterContent(std::vector<DocumentElement *> * pFooterContent) { mpFooterContent = pFooterContent; }
	void setHeaderLeftContent(std::vector<DocumentElement *> * pHeaderContent) { mpHeaderLeftContent = pHeaderContent; }
	void setFooterLeftContent(std::vector<DocumentElement *> * pFooterContent) { mpFooterLeftContent = pFooterContent; }
protected:
	void _writeHeaderFooter(const char *headerFooterTagName, const std::vector<DocumentElement *> & headerFooterContent,
				DocumentHandler &xHandler) const;
private:
        WPXPropertyList mxPropList;
	std::vector<DocumentElement *> * mpHeaderContent;
	std::vector<DocumentElement *> * mpFooterContent;
	std::vector<DocumentElement *> * mpHeaderLeftContent;
	std::vector<DocumentElement *> * mpFooterLeftContent;
};
#endif
