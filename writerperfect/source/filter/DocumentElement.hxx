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

#ifndef _DOCUMENTELEMENT_H
#define _DOCUMENTELEMENT_H
#include <libwpd/libwpd.h>
#include <libwpd/WPXProperty.h>
#include <libwpd/WPXString.h>
#include <vector>

#include "DocumentHandler.hxx"

class DocumentElement
{
public:	
	virtual ~DocumentElement() {}
	virtual void write(DocumentHandler &xHandler) const = 0;
	virtual void print() const {}
};

class TagElement : public DocumentElement
{
public:
	TagElement(const char *szTagName) : msTagName(szTagName) {}
	const WPXString & getTagName() const { return msTagName; }
	virtual void print() const;
private:
	WPXString msTagName;
};

class TagOpenElement : public TagElement
{
public:
	TagOpenElement(const char *szTagName) : TagElement(szTagName) {}
	~TagOpenElement() {}
	void addAttribute(const char *szAttributeName, const WPXString &sAttributeValue);
	virtual void write(DocumentHandler &xHandler) const;
	virtual void print () const;
private:
	WPXPropertyList maAttrList;
};

class TagCloseElement : public TagElement
{
public:
	TagCloseElement(const char *szTagName) : TagElement(szTagName) {}
	virtual void write(DocumentHandler &xHandler) const;
};

class CharDataElement : public DocumentElement
{
public:
	CharDataElement(const char *sData) : DocumentElement(), msData(sData) {}
	virtual void write(DocumentHandler &xHandler) const;
private:
	WPXString msData;
};

class TextElement : public DocumentElement
{
public:
	TextElement(const WPXString & sTextBuf);
	virtual void write(DocumentHandler &xHandler) const;

private:
	WPXString msTextBuf;
};
 
#endif
