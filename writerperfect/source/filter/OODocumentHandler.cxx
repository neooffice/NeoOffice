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
 *  Copyright (C) 2004 William Lachance (william.lachance@sympatico.ca)
 *  Copyright (C) 2004 Net Integration Technologies (http://www.net-itech.com)
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
 *  Modified February 2005 by Patrick Luby. SISSL Removed. NeoOffice is
 *  distributed under GPL only under modification term 3 of the LGPL.
 *
 *  Contributor(s): _______________________________________
 *
 ************************************************************************/

#include "OODocumentHandler.hxx"

#ifdef MACOSX
#include <string.h>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif

#ifndef _ATTRLIST_HPP_
#include <xmloff/attrlist.hxx>
#endif

using namespace ::rtl;
using rtl::OUString;

using com::sun::star::xml::sax::XAttributeList;

OODocumentHandler::OODocumentHandler(Reference < XDocumentHandler > &xHandler) :
        mxHandler(xHandler)
{
}

void OODocumentHandler::startDocument() 
{
	mxHandler->startDocument();
}

void OODocumentHandler::endDocument()
{
	mxHandler->endDocument();
}

void OODocumentHandler::startElement(const char *psName, const WPXPropertyList &xPropList)
{
        SvXMLAttributeList *pAttrList = new SvXMLAttributeList();
	Reference < XAttributeList > xAttrList(pAttrList);
	WPXPropertyList::Iter i(xPropList);
	for (i.rewind(); i.next(); )
	{
                // filter out libwpd elements
                if (strlen(i.key()) > 6 && strcmp(i.key(), "libwpd") != 0)
                        pAttrList->AddAttribute(OUString::createFromAscii(i.key()),
                                                OUString::createFromAscii(i()->getStr().cstr()));
        }

        mxHandler->startElement(OUString::createFromAscii(psName), xAttrList);
}

void OODocumentHandler::endElement(const char *psName)
{
        mxHandler->endElement(OUString::createFromAscii(psName));
}

void OODocumentHandler::characters(const WPXString &sCharacters)
{
        OUString sCharU16(sCharacters.cstr(), strlen(sCharacters.cstr()), RTL_TEXTENCODING_UTF8);
        mxHandler->characters(sCharU16);
}
