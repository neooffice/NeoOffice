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


#ifndef _PAGESPAN_H
#define _PAGESPAN_H
#include <libwpd/libwpd.h>

#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

using com::sun::star::uno::Reference;
using com::sun::star::xml::sax::XDocumentHandler;

class DocumentElement;

class PageSpan
{
public:
	PageSpan(const int iSpan, const float fFormLength, const float fFormWidth, const WPXFormOrientation fFormOrientation,
			 const float fLeftMargin, const float fRightMargin, const float fTopMargin, const float fBottomMargin);
	virtual ~PageSpan();
	void writePageMaster(const int iNum, Reference < XDocumentHandler > &xHandler) const;
	void writeMasterPages(const int iStartingNum, const int iPageMasterNum, const bool bLastPageSpan, Reference < XDocumentHandler > &xHandler) const;
	const int getSpan() const { return miSpan; }
	float getFormLength() { return mfFormLength; }
	float getFormWidth() { return mfFormWidth; }
	WPXFormOrientation getFormOrientation() { return mfFormOrientation; }
	float getMarginLeft() { return mfMarginLeft; }
	float getMarginRight() { return mfMarginRight; }

	const vector<DocumentElement *> * getHeaderContent() const { return mpHeaderContent; }
	void setHeaderContent(vector<DocumentElement *> * pHeaderContent) { mpHeaderContent = pHeaderContent; }
	void setFooterContent(vector<DocumentElement *> * pFooterContent) { mpFooterContent = pFooterContent; }
	void setHeaderLeftContent(vector<DocumentElement *> * pHeaderContent) { mpHeaderLeftContent = pHeaderContent; }
	void setFooterLeftContent(vector<DocumentElement *> * pFooterContent) { mpFooterLeftContent = pFooterContent; }
protected:
	void _writeHeaderFooter(const char *headerFooterTagName, const vector<DocumentElement *> & headerFooterContent,
				Reference < XDocumentHandler > &xHandler) const;
private:
	int miSpan;
	float mfFormLength, mfFormWidth, mfMarginLeft, mfMarginRight, mfMarginTop, mfMarginBottom;
	WPXFormOrientation mfFormOrientation;
	vector<DocumentElement *> * mpHeaderContent;
	vector<DocumentElement *> * mpFooterContent;
	vector<DocumentElement *> * mpHeaderLeftContent;
	vector<DocumentElement *> * mpFooterLeftContent;
};
#endif
