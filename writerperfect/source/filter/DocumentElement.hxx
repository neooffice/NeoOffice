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


#ifndef _DOCUMENTELEMENT_H
#define _DOCUMENTELEMENT_H
#include <libwpd/libwpd.h>
#include <libwpd/libwpd_support.h>
#include <vector>

using namespace std;

#include "WordPerfectCollector.hxx"
#include "TextRunStyle.hxx"
#include "SectionStyle.hxx"
#include "TableStyle.hxx"

#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

#include <xmloff/attrlist.hxx>

class DocumentElement
{
public:	
	virtual ~DocumentElement() {}
	virtual void write(Reference < XDocumentHandler > &xHandler) const = 0;
	virtual void print() const {}
};

class TagElement : public DocumentElement
{
public:
	TagElement(const char *szTagName) : msTagName(szTagName) {}
	const UTF8String & getTagName() const { return msTagName; }
	virtual void print() const;
private:
	UTF8String msTagName;
};

class TagOpenElement : public TagElement
{
public:
	TagOpenElement(const char *szTagName) : TagElement(szTagName) {}
	~TagOpenElement() {}
	void addAttribute(const char *szAttributeName, const char *szAttributeValue);
	virtual void write(Reference < XDocumentHandler > &xHandler) const;
	virtual void print () const;
private:
	SvXMLAttributeList maAttrList;
};

class TagCloseElement : public TagElement
{
public:
	TagCloseElement(const char *szTagName) : TagElement(szTagName) {}
	virtual void write(Reference < XDocumentHandler > &xHandler) const;
};

class CharDataElement : public DocumentElement
{
public:
	CharDataElement(const char *sData) : DocumentElement(), msData(sData) {}
	virtual void write(Reference < XDocumentHandler > &xHandler) const;
private:
	UTF8String msData;
};

class TextElement : public DocumentElement
{
public:
	TextElement(const UCSString & sTextBuf);
	virtual void write(Reference < XDocumentHandler > &xHandler) const;

private:
	UCSString msTextBuf;
};
 
#endif
